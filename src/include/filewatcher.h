#pragma once

#include <vector>
#include <string>

#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include <unordered_map>
#include <thread>
#include <stdexcept>
#include <mutex>
#include <atomic>

#include "error/result.h"
#include "error/error.h"
#include "task_runner.h"
#include "types.h"

namespace flowhook
{
    class FileWatcher
    {
    private:
        int inotify_fd = -1;
        int poll_num;
        std::unordered_map<int, std::string> watch_registry;
        std::unordered_map<std::string, int> r_watch_registry;
        struct pollfd fd[1];
        nfds_t nfds;

        std::unordered_map<uint32_t, std::vector<WatchCallback>> event_callbacks;
        int next_callback_id = 0;

        std::mutex registry_mutex;
        std::atomic<bool> isWatching{false};
        std::thread background_thread;

        Result<WatchEvent> handle_events(int fd, std::vector<int> wd, int argc);
        Result<void> event_loop(int timeout);

        FileWatcher() = default;

    public:
        // Factory Function constuctor
        Result<void> init();

        // destructor
        ~FileWatcher()
        {
            if(inotify_fd != -1)
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