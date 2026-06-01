#pragma once

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
#include <functional>
#include <thread>
#include <stdexcept>
#include <mutex>
#include <atomic>

#include "error/result.h"
#include "error/error.h"

namespace l_fw
{
    struct _i_event
    {
        int wd;
        std::string filetype;
        std::string path;
        uint32_t event_mask;
    };

    class FileWatcher
    {
    private:
        int inotify_fd, poll_num;
        std::unordered_map<int, std::string> watch_registry;
        std::unordered_map<std::string, int> r_watch_registry;
        struct pollfd fd[1];
        nfds_t nfds;

        using WatchCallback = void(*)(_i_event);

        std::unordered_map<uint32_t, std::vector<WatchCallback>> event_callbacks;
        int next_callback_id = 0;

        std::mutex registry_mutex;
        std::atomic<bool> isWatching{false};
        std::thread background_thread;

        Result<_i_event> handle_events(int fd, std::vector<int> wd, int argc);
        Result<void> event_loop(int timeout);

    public:
        // constructor
        FileWatcher()
        {
            inotify_fd = inotify_init1(IN_NONBLOCK);
            if (inotify_fd == -1)
            {
                throw std::runtime_error("inotify_init1 failed");
            }
            nfds = 1;
            fd[0].fd = inotify_fd;
            fd[0].events = POLLIN;
        }

        // destructor
        ~FileWatcher()
        {
            close(inotify_fd);
        }

        // prevent copy
        FileWatcher(const FileWatcher &) = delete;
        FileWatcher &operator=(const FileWatcher &) = delete;
        bool is_running() const { return isWatching; }

        Result<void> add_path(std::string &arg);
        Result<void> remove_path(std::string &arg);
        Result<void> start(int timeout);
        Result<void> stop();

        Result<void> link_event(uint32_t event_mask, WatchCallback callback);
        Result<void> unlink_event(uint32_t event_mask, WatchCallback callback);
        Result<std::vector<std::string>> get_watch_list();
    };
}