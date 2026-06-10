#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <source_location>

namespace flowhook
{
    enum class ErrorCode
    {
        // Task life cycls
        TASK_NOT_FOUND,
        TASK_ALREADY_RUNNING,
        TASK_NOT_RUNNING,
        TASK_FULL,
        TASK_NOT_ACTIVE,
        TASK_ALREADY_ACTIVE,

        // Config lookup errors
        EVENT_NOT_FOUND,
        COMMAND_NOT_FOUND,
        PATH_NOT_FOUND,
        DUPLICATE_ENTRY,

        // System/ OS Errors
        SYS_POLL_FAILED,
        SYS_PIPE_FAILED,
        SYS_IO_FAILED,

        UNKNOWN
    };

    struct FWError
    {
        ErrorCode code;
        std::string message;
        
        FWError(ErrorCode code, std::string message) : code(code), message(message) {}

        static FWError make(ErrorCode code, std::string message)
        {
            FWError e(code, message);
            return e;
        }
    };
}
