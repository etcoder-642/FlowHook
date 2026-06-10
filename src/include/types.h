#include <string>
#include <vector>

#include "error/result.h"
#include "task_runner.h"

namespace flowhook 
{
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