#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <source_location>

namespace l_fw
{
    enum class ErrorCode
    {
        EVENT_NOT_FOUND,
        COMMAND_NOT_FOUND,
        EXISTING_VALUE,
        ALREADY_RUNNING,
        NOT_RUNNING,
        POLL_ERR,
        PIPE_ERR,
        SYSTEM_IO_ERROR,
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
