#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>

#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <map>
#include <unordered_map>

#include <cstring>

#include <mutex>
#include <thread>

#include "../include/filewatcher.h"
#include "../include/macros.hpp"

using namespace flowhook;
using namespace std;

Result<void> FileWatcher::init()
{
    inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd == -1)
    {
        return Result<void>::Err(FWError::make(
            ErrorCode::SYS_IO_FAILED,
            "Error: inotify_init1 failure"));
    }
    nfds = 1;
    fd[0].fd = inotify_fd;
    fd[0].events = POLLIN;
}

Result<WatchEvent> FileWatcher::handle_events(int fd, vector<int> wd, int argc)
{
    cout << "[FLOWHOOK] FileWatcher::handle_events:: handling detected events... " << endl;
    WatchEvent e;
    const struct inotify_event *event;
    char buffer[4096];
    ssize_t len;

    // drain event queue from buffer
    for (;;)
    {
        len = read(fd, buffer, sizeof(buffer));
        if (len == -1 && errno != EAGAIN)
        {
            std::string _e_msg = "Error: read failure; errno: " + std::string(strerror(errno));
            return Result<WatchEvent>::Err(FWError::make(
                ErrorCode::SYS_IO_FAILED, _e_msg));
        }

        if (len <= 0)
            break;

        int i = 0;
        for (char *ptr = buffer; ptr < buffer + len;)
        {
            event = reinterpret_cast<const struct inotify_event *>(ptr);

            cout << "[FLOWHOOK]::handle_events:: event mask: " << event->mask << endl;

            if (event->mask & IN_CLOSE_WRITE)
            {
                e.event_mask = IN_CLOSE_WRITE;
            }
            else if (event->mask & IN_MODIFY)
            {
                e.event_mask = IN_MODIFY;
            }

            string base_path = watch_registry[event->wd];
            if (event->len > 0)
            {
                e.path = base_path + "/" + event->name;
            }
            else
            {
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
    return Result<WatchEvent>::Err(ErrorCode::EVENT_NOT_FOUND, "Error: empty event");
}

Result<void> FileWatcher::add_path(string &arg)
{
    lock_guard<mutex> lock(registry_mutex);
    int wd = inotify_add_watch(inotify_fd, arg.c_str(),
                               IN_MOVED_TO | IN_MOVED_FROM | IN_MODIFY | IN_CLOSE_WRITE);
    if (wd == -1)
        return Result<void>::Err(FWError::make(
            ErrorCode::SYS_IO_FAILED,
            "Error: inotify_add_watch failure"));
    watch_registry[wd] = arg;
    r_watch_registry[arg] = wd;
    return Result<void>::Ok();
}

Result<void> FileWatcher::remove_path(string &arg)
{
    lock_guard<mutex> lock(registry_mutex);
    int _r_wd = r_watch_registry[arg];
    inotify_rm_watch(inotify_fd, _r_wd);
    watch_registry.erase(_r_wd);
    r_watch_registry.erase(arg);
    return Result<void>::Ok();
}

Result<void> FileWatcher::link_event(uint32_t event_mask, WatchCallback callback)
{
    lock_guard<mutex> lock(registry_mutex);
    vector<WatchCallback> temp_cb = event_callbacks[event_mask];
    cout << "[FLOWHOOK] Linking event " << event_mask << " to callback " << endl;
    for (auto &cb : temp_cb)
    {
        if (cb == callback)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::DUPLICATE_ENTRY,
                "Error: event already linked with callback"));
        }
    }
    event_callbacks[event_mask].push_back(callback);
    return Result<void>::Ok();
}

Result<void> FileWatcher::unlink_event(uint32_t event_mask, WatchCallback callback)
{
    if (event_callbacks.find(event_mask) == event_callbacks.end())
    {
        return Result<void>::Err(FWError::make(
            ErrorCode::EVENT_NOT_FOUND,
            "Error: no such event"));
    }

    for (auto it = event_callbacks[event_mask].begin(); it != event_callbacks[event_mask].end(); it++)
    {
        if (*it == callback)
        {
            event_callbacks[event_mask].erase(it);
            return Result<void>::Ok();
        }
    }
    return Result<void>::Err(FWError::make(
        ErrorCode::CALLBACK_NOT_FOUND, "Error: callback not found"));
}

Result<void> FileWatcher::event_loop(int timeout)
{
    cout << "[FLOWHOOK] FileWatcher:BackgroundThread event poll starting... " << endl;
    while (isWatching)
    {
        // cout << "[FLOWHOOK]:FileWatcher:BackgroundThread Polling... " << endl;
        poll_num = poll(fd, nfds, timeout);

        if (poll_num == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_POLL_FAILED,
                    "Error: polling error"));
            }
        }

        if (poll_num < 0)
            continue;

        if (fd[0].revents & POLLIN)
        {
            WatchEvent e;
            vector<WatchCallback> callback;
            {
                lock_guard<mutex> lock(registry_mutex);

                vector<int> _wd_keys;
                for (const auto &[wd, path] : watch_registry)
                {
                    _wd_keys.push_back(wd);
                }
                cout << "[FLOWHOOK] Handle Events... " << endl;
                auto e = TRY(handle_events(fd[0].fd, _wd_keys, watch_registry.size()), void);
                callback = event_callbacks[e.event_mask];
            }
            // debounce check
            cout << "[FLOWHOOK]:FileWatcher:BackgroundThread Debouncing... " << endl;

            cout << "[FLOWHOOK] callback size: " << callback.size() << endl;
            if (!callback.empty())
            {
                cout << "[FLOWHOOK]::event_loop:: e.event_mask = " << e.event_mask << endl;
                for (auto &cb : callback)
                {
                    cout << "[FLOWHOOK]:FileWatcher:BackgroundThread Calling Callback... " << endl;
                    TEST(cb.invoke(e));
                }
            }
        }
    }
    return Result<void>::Ok();
}

Result<void> FileWatcher::start(int timeout)
{
    if (timeout < 10)
    {
        timeout = 10;
    }

    if (isWatching)
    {
        return Result<void>::Err(FWError::make(
            ErrorCode::FILEWATCHER_ALREADY_RUNNING,
            "Error: file watcher already running"));
    }
    try
    {
        isWatching = true;
        cout << "[FLOWHOOK]:FileWatcher Starting Background Thread... " << endl;
        background_thread = std::thread(&FileWatcher::event_loop, this, timeout);
    }
    catch (std::system_error &e)
    {
        return Result<void>::Err(FWError::make(
            ErrorCode::SYS_THREAD_FAILED,
            "Error: starting background thread failed"));
    }
    return Result<void>::Ok();
}

Result<void> FileWatcher::stop()
{
    if (!isWatching)
    {
        return Result<void>::Err(FWError::make(
            ErrorCode::FILEWATCHER_NOT_RUNNING,
            "Error: file watcher not running"));
    }
    isWatching = false;
    if (background_thread.joinable())
    {
        background_thread.join();
    }
    return Result<void>::Ok();
}

Result<vector<string>> FileWatcher::get_watch_list()
{
    lock_guard<mutex> lock(registry_mutex);
    vector<string> list;
    for (auto &[wd, path] : watch_registry)
    {
        list.push_back(path);
    }
    if (list.empty())
    {
        return Result<vector<string>>::Err(FWError::make(
            ErrorCode::FILEWATCHER_EMPTY,
            "Error: no registered paths"));
    }
    return Result<vector<string>>::Ok(list);
}