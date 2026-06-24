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
        int wd = -1;
        std::string filetype = "";
        std::string path = "";
        uint32_t event_mask = -1;

        bool isNull() const
        {
            return wd == -1 && filetype.empty() && path.empty() && event_mask == -1;
        }
        WatchEvent(int wd, std::string filetype, std::string path, uint32_t event_mask)
            : wd(wd), filetype(filetype), path(path), event_mask(event_mask) {}

        WatchEvent(): wd(-1), filetype(""), path(""), event_mask(-1) {}
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

        bool isNull() const
        {
            return ptr == nullptr && handler == nullptr && raw_callback == nullptr;
        }
    };

    struct Task
    {
        std::string name = "";
        std::string working_directory = "";
        int watching_depth = 1;
        std::vector<std::string> commands = {};
        std::vector<std::string> paths = {};

        std::vector<std::string> ignored_patterns = {};
        std::vector<std::string> ignored_paths = {};

        std::vector<std::string> on_success = {};
        std::vector<std::string> on_failure = {};
        bool isActive = false;
        bool isRunning = false;

        bool isNull() const {
            return name.empty() && working_directory.empty() && commands.empty() && paths.empty() &&
                on_success.empty() && on_failure.empty() && !isActive && !isRunning && watching_depth == 1 &&
                ignored_paths.empty() && ignored_patterns.empty();
        }
    };

    struct ExecutionResult
    {
        int id;
        int exit_code;
        WatchEvent _event;
        std::string log;
        std::vector<std::string> build_commands;

        ExecutionResult(int id, int exit_code, WatchEvent event = WatchEvent(), std::string log = "", std::vector<std::string> build_commands = std::vector<std::string>())
            : id(id), exit_code(exit_code), log(log), build_commands(build_commands)
        {
            _event = event;
        }
        ExecutionResult(): id(-1), exit_code(-1), log(""), build_commands(std::vector<std::string>()) {}
    };

}
