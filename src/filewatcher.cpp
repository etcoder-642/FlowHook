#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <map>
#include <unordered_map>
#include <stdexcept>

#include <mutex>
#include <thread>

#include "./filewatcher.h"

using namespace l_fw;
using namespace std;

_i_event FileWatcher::handle_events(int fd, vector<int> wd, int argc)
{
    _i_event e;
    const struct inotify_event *event;
    char buffer[4096];
    ssize_t len;

    for (;;)
    {
        len = read(fd, buffer, sizeof(buffer));
        if (len == -1 && errno != EAGAIN)
        {
            throw runtime_error("read failed");
        }

        if (len <= 0)
            break;

        int i = 0;
        for (char *ptr = buffer; ptr < buffer + len;)
        {
            event = reinterpret_cast<const struct inotify_event *>(ptr);

            if (event->mask & IN_CREATE)
            {
                e.event_mask = IN_CREATE;
            }
            else if (event->mask & IN_DELETE)
            {
                e.event_mask = IN_DELETE;
            }
            else if (event->mask & IN_ACCESS)
            {
                e.event_mask = IN_ACCESS;
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
            return e;

            ptr += sizeof(struct inotify_event) + event->len;
            if (i <= argc)
                i++;
        }
    }
    return {};
}

bool FileWatcher::add_path(string &arg)
{
    lock_guard<mutex> lock(registry_mutex);
    int wd = inotify_add_watch(inotify_fd, arg.c_str(),
                               IN_ACCESS | IN_CREATE | IN_DELETE | IN_MODIFY);
    if (wd == -1)
        return false;
    watch_registry[wd] = arg;
    r_watch_registry[arg] = wd;
    return true;
}

bool FileWatcher::remove_path(string &arg)
{
    lock_guard<mutex> lock(registry_mutex);
    int _r_wd = r_watch_registry[arg];
    inotify_rm_watch(inotify_fd, _r_wd);
    watch_registry.erase(_r_wd);
    r_watch_registry.erase(arg);
    return true;
}

void FileWatcher::on_event(uint32_t event_mask, WatchCallback callback)
{
    lock_guard<mutex> lock(registry_mutex);
    event_callbacks[event_mask] = callback;
}

void FileWatcher::event_loop(int timeout)
{
    while (isWatching)
    {
        poll_num = poll(fd, nfds, timeout);

        if (poll_num == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                throw runtime_error("Poll Error");
            }
        }

        if (poll_num < 0)
            continue;

        if (fd[0].revents & POLLIN)
        {
            _i_event e;
            WatchCallback callback;
            {
                lock_guard<mutex> lock(registry_mutex);

                vector<int> _wd_keys;
                for (const auto &[wd, path] : watch_registry)
                {
                    _wd_keys.push_back(wd);
                }
                e = handle_events(fd[0].fd, _wd_keys, watch_registry.size());
                callback = event_callbacks[e.event_mask];
            }
            if (callback)
            {
                callback(e);
            }
        }
    }
}

void FileWatcher::start(int timeout)
{
    if (timeout < 10)
    {
        timeout = 10;
    }

    isWatching = true;
    background_thread = std::thread(&FileWatcher::event_loop, this, timeout);
    return;
}

void FileWatcher::stop()
{
    isWatching = false;
    if (background_thread.joinable())
    {
        background_thread.join();
    }
}
