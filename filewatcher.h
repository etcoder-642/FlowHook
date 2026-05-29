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

namespace l_fw
{
    struct _i_event {
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

        using WatchCallback = std::function<void(_i_event)>;
        std::unordered_map<uint32_t, WatchCallback> event_callbacks;

        std::mutex registry_mutex;
        std::atomic<bool> isWatching{false};
        std::thread background_thread;

        _i_event handle_events(int fd, std::vector<int> wd, int argc);
        void event_loop(int timeout);

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

        bool add_path(std::string &arg);
        bool remove_path(std::string &arg);
        void start_polling();
        void start(int timeout);
        void stop();

        void on_event(uint32_t event_mask, WatchCallback callback);
    };
}