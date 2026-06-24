#include <filesystem>
#include <iostream>
#include <string>
#include <sys/inotify.h>
#include <unistd.h>
#include <vector>

#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <unordered_map>

#include <cstring>

#include <mutex>
#include <thread>
#include <fnmatch.h>

#include "include/filewatcher.h"
#include "error/result.h"
#include "include/macros.hpp"

using namespace flowhook;
using namespace std;
namespace fs = std::filesystem;

Result<FileWatcher *> FileWatcher::create() {
  FileWatcher *fw = new FileWatcher();
  TEST_OVERLOADED(fw->init(), FileWatcher *);
  return Result<FileWatcher *>::Ok(fw);
}

Result<void> FileWatcher::init() {
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED,
                                           "Error: inotify_init1 failure"));
  }
  nfds = 1;
  fd[0].fd = inotify_fd;
  fd[0].events = POLLIN;
  return Result<void>::Ok();
}

Result<WatchEvent> FileWatcher::handle_events(int fd, vector<int> wd,
                                              int argc) {
  WatchEvent e;
  const struct inotify_event *event;
  char buffer[4096];
  ssize_t len;

  // drain event queue from buffer
  for (;;) {
    len = read(fd, buffer, sizeof(buffer));
    if (len == -1 && errno != EAGAIN) {
      std::string _e_msg =
          "Error: read failure; errno: " + std::string(strerror(errno));
      return Result<WatchEvent>::Err(
          FWError::make(ErrorCode::SYS_IO_FAILED, _e_msg));
    }

    if (len <= 0)
      break;

    int i = 0;
    for (char *ptr = buffer; ptr < buffer + len;) {
      event = reinterpret_cast<const struct inotify_event *>(ptr);

      if (event->mask & IN_CLOSE_WRITE) {
        FW_LOG("[FLOWHOOK] - EVENT detected - IN_CLOSE_WRITE");
        e.event_mask = IN_CLOSE_WRITE;
      } else if (event->mask & IN_MODIFY) {
        FW_LOG("[FLOWHOOK] - EVENT detected - IN_MODIFY");
        e.event_mask = IN_MODIFY;
      } else if (event->mask & IN_MOVED_TO) {
        FW_LOG("[FLOWHOOK] - EVENT detected - IN_MOVED_TO");
        e.event_mask = IN_MOVED_TO;
      } else if (event->mask & IN_MOVED_FROM) {
        FW_LOG("[FLOWHOOK] - EVENT detected - IN_MOVED_FROM");
        e.event_mask = IN_MOVED_FROM;
      } else {
        FW_LOG("[FLOWHOOK] - EVENT detected " << event->mask);
      }

      string base_path = watch_registry[event->wd];
      if (event->len > 0) {
        e.path = base_path + "/" + event->name;
      } else {
        e.path = base_path;
      }
      e.wd = event->wd;

      if (event->mask & IN_ISDIR)
        e.filetype = "dir";
      else
        e.filetype = "file";

      ptr += sizeof(struct inotify_event) + event->len;
      if (i <= argc)
        i++;
      return Result<WatchEvent>::Ok(e);
    }
  }
  return Result<WatchEvent>::Err(ErrorCode::EVENT_NOT_FOUND,
                                 "Error: empty event");
}

Result<void> FileWatcher::add_ignored_path(const string &path)
{
    lock_guard<mutex> lock(registry_mutex);
    ignored_paths.push_back(path);

    vector<string> to_remove = {};
    for(auto [wd, watched_path]: watch_registry)
    {
        if(isIgnored(watched_path))
            to_remove.push_back(watched_path);
    }
    for(auto p: to_remove)
       TEST(remove_path_internal(p));

    return Result<void>::Ok();
}

Result<void> FileWatcher::add_ignored_pattern(const string &pattern)
{
    lock_guard<mutex> lock(registry_mutex);
    ignored_patterns.push_back(pattern);
    vector<string> to_remove = {};
    for(auto [wd, watched_path]: watch_registry)
    {
        if(isIgnored(watched_path))
            to_remove.push_back(watched_path);
    }
    for(auto p: to_remove)
       TEST(remove_path_internal(p));

    return Result<void>::Ok();
}

bool FileWatcher::isIgnored(const string &path)
{
    for(auto &p: ignored_paths)
    {
        if(p == path) return true;
    }

    string filename = fs::path(path).filename().string();
    for(auto &p: ignored_patterns)
    {
        if(fnmatch(p.c_str(), filename.c_str(), 0) == 0) return true;
    }

    return false;
}

Result<void> FileWatcher::add_path(const string &arg, int depth) {
  lock_guard<mutex> lock(registry_mutex);
  TEST(add_path_internal(arg, depth, 0));
  return Result<void>::Ok();
}

Result<void> FileWatcher::add_path_internal(const string &arg, int MAX_DEPTH, int CURRENT_DEPTH)
{
    // if path is directory add each files inside it iteratively
    if (fs::is_directory(arg)) {
      for (auto &entry : fs::directory_iterator(arg)) {
          if(entry.is_regular_file()){
              if(isIgnored(entry.path().string())) continue;

              int wd = inotify_add_watch(inotify_fd, entry.path().c_str(),
                                         IN_MOVED_TO | IN_MOVED_FROM | IN_MODIFY |
                                             IN_CLOSE_WRITE);
              if (wd == -1)
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_IO_FAILED, "Error: inotify_add_watch failure"));
              watch_registry[wd] = entry.path();
              r_watch_registry[entry.path()] = wd;
          } else if(entry.is_directory() && MAX_DEPTH > CURRENT_DEPTH) {
              TEST(add_path_internal(entry.path(), MAX_DEPTH, CURRENT_DEPTH + 1));
          }
      }
      return Result<void>::Ok();
    }
    FW_LOG("[DEBUG] inotify_add_watch on: " << arg);
    // check if path already exists
    for (auto [w, p] : watch_registry) {
      if (arg == p) {
        return Result<void>::Err(FWError::make(ErrorCode::PATH_ALREADY_EXISTS,
                                               "Error: Path already exists"));
      }
    }
    // check if path is empty
    if (arg.empty()) {
      return Result<void>::Err(
          FWError::make(ErrorCode::EMPTY_VALUE, "Error: path is empty"));
    }

    // add path to inotify
    int wd = inotify_add_watch(inotify_fd, arg.c_str(),
                               IN_MOVED_TO | IN_MOVED_FROM | IN_MODIFY |
                                   IN_CLOSE_WRITE);
    if (wd == -1)
      return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED,
                                             "Error: inotify_add_watch failure"));
    watch_registry[wd] = arg;
    r_watch_registry[arg] = wd;
    return Result<void>::Ok();
}

Result<void> FileWatcher::remove_path(const string &arg) {
  lock_guard<mutex> lock(registry_mutex);
  return remove_path_internal(arg);
}

Result<void> FileWatcher::remove_path_internal(const string &arg)
{
    // check if path exists
    bool path_exists = false;
    for (auto [w, p] : watch_registry) {
      if (arg == p) {
        path_exists = true;
      }
    }
    if (!path_exists) {
      return Result<void>::Err(
          FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: Path not found"));
    }

    // remove path from inotify
    int _r_wd = r_watch_registry[arg];
    inotify_rm_watch(inotify_fd, _r_wd);
    watch_registry.erase(_r_wd);
    r_watch_registry.erase(arg);
    return Result<void>::Ok();
}

Result<void> FileWatcher::link_event(uint32_t event_mask,
                                     WatchCallback callback) {
  lock_guard<mutex> lock(registry_mutex);
  if (event_mask != IN_MODIFY && event_mask != IN_CLOSE_WRITE &&
      event_mask != IN_MOVED_TO && event_mask != IN_MOVED_FROM) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EVENT_NOT_SUPPORTED, "Error: no such event"));
  }

  // check if callback already exists
  vector<WatchCallback> temp_cb = event_callbacks[event_mask];
  for (auto &cb : temp_cb) {
    if (cb == callback) {
      return Result<void>::Err(
          FWError::make(ErrorCode::DUPLICATE_ENTRY,
                        "Error: event already linked with callback"));
    }
  }
  event_callbacks[event_mask].push_back(callback);
  return Result<void>::Ok();
}

Result<void> FileWatcher::unlink_event(uint32_t event_mask,
                                       WatchCallback callback) {
  if (event_callbacks.find(event_mask) == event_callbacks.end()) {
    return Result<void>::Err(
        FWError::make(ErrorCode::EVENT_NOT_FOUND, "Error: no such event"));
  }

  for (auto it = event_callbacks[event_mask].begin();
       it != event_callbacks[event_mask].end(); it++) {
    if (*it == callback) {
      event_callbacks[event_mask].erase(it);
      return Result<void>::Ok();
    }
  }
  return Result<void>::Err(FWError::make(ErrorCode::CALLBACK_NOT_FOUND,
                                         "Error: callback not found"));
}

Result<void> FileWatcher::event_loop(int timeout) {
  auto last_event_time = std::chrono::steady_clock::now();
  const auto debounce_ms = std::chrono::milliseconds(300);

  while (isWatching) {
    poll_num = poll(fd, nfds, timeout);

    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      else {
        return Result<void>::Err(
            FWError::make(ErrorCode::SYS_POLL_FAILED, "Error: polling error"));
      }
    }

    if (poll_num < 0)
      continue;

    if (fd[0].revents & POLLIN) {
      auto now = std::chrono::steady_clock::now();
      if (now - last_event_time < debounce_ms)
        continue;
      last_event_time = now;

      FW_LOG("[DEBUG] POLLIN received");
      WatchEvent e;
      vector<WatchCallback> callback;
      {
        lock_guard<mutex> lock(registry_mutex);

        vector<int> _wd_keys;
        for (const auto &[wd, path] : watch_registry) {
          _wd_keys.push_back(wd);
        }
        e = TRY(handle_events(fd[0].fd, _wd_keys, watch_registry.size()), void);
        callback = event_callbacks[e.event_mask];
      }
      if (!callback.empty()) {
        for (auto &cb : callback) {
          TEST(cb.invoke(e));
        }
      }
    }
  }
  return Result<void>::Ok();
}

Result<void> FileWatcher::start(int timeout) {
  FW_LOG("[FLOWHOOK] - filewatcher started...");
  if (timeout < 10) {
    timeout = 10;
  }

  if (isWatching) {
    return Result<void>::Err(
        FWError::make(ErrorCode::FILEWATCHER_ALREADY_RUNNING,
                      "Error: file watcher already running"));
  }
  try {
    FW_LOG("[FLOWHOOK] - launching a background thread to watch files ..."
        );
    isWatching = true;
    background_thread = std::thread(&FileWatcher::event_loop, this, timeout);
  } catch (std::system_error &e) {

    return Result<void>::Err(
        FWError::make(ErrorCode::SYS_THREAD_FAILED,
                      "Error: starting background thread failed"));
  }
  return Result<void>::Ok();
}

Result<void> FileWatcher::stop() {
  if (!isWatching) {
    return Result<void>::Err(FWError::make(ErrorCode::FILEWATCHER_NOT_RUNNING,
                                           "Error: file watcher not running"));
  }
  isWatching = false;
  if (background_thread.joinable()) {
    background_thread.join();
  }
  return Result<void>::Ok();
}

Result<vector<string>> FileWatcher::get_watch_list() {
  lock_guard<mutex> lock(registry_mutex);
  vector<string> list;
  for (auto &[wd, path] : watch_registry) {
    list.push_back(path);
  }
  return Result<vector<string>>::Ok(list);
}
