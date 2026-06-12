#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "error/result.h"

namespace flowhook
{
    class TaskRunner;
    struct WatchEvent
    {
        int wd;
        std::string filetype;
        std::string path;
        uint32_t event_mask;
    };

    struct WatchCallback
    {
        TaskRunner *ptr;
        Result<void> (TaskRunner::*handler)(const WatchEvent &e) = nullptr;
        Result<void> (*raw_callback)(const WatchEvent &e) = nullptr;
        
        static WatchCallback from_raw(Result<void> (*fn)(const WatchEvent &e))
        {
            WatchCallback cb;
            cb.ptr = nullptr;
            cb.handler = nullptr;
            cb.raw_callback = fn;
            return cb;
        }
        bool operator==(const WatchCallback &o) const
        {
            return ptr == o.ptr && handler == o.handler && raw_callback == o.raw_callback;
        }

        Result<void> invoke(const WatchEvent &e) const
        {
            if (ptr == nullptr)
            {
                return raw_callback(e);
            }
            else
            {
                return (ptr->*handler)(e);
            }
        }
    };

    struct Task
    {
        std::string name;
        std::string working_directory;
        std::vector<std::string> commands;
        std::vector<std::string> paths;

        std::vector<std::string> on_success;
        std::vector<std::string> on_failure;
        bool isActive;
        bool isRunning = false;
    };

    struct ExecutionResult
    {
        int id;
        int exit_code;
        WatchEvent _event;
        std::string log;
        std::vector<std::string> build_commands;
    };

}