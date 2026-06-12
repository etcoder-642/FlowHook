#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <sstream>
#include <ctime>
#include <filesystem>

#include "include/session_logger.h"
#include "include/macros.hpp"

namespace fs = std::filesystem;
// Error
using namespace std;
namespace flowhook
{

    SessionLogger::SessionLogger()
    {
        is_running = false;
        flushed = false;
    }

    SessionLogger::~SessionLogger()
    {
        auto _temp = stop();
    }

    Result<void> SessionLogger::start(const string &file_path)
    {
        // check if session logger is already running
        if (is_running)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SESSION_LOGGER_ALREADY_RUNNING, "Error: session logger already running"));
        }
        // check if file path is empty
        if (file_path.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: file path is empty"));
        }
        // check if file path is a valid path
        if (!fs::exists(file_path))
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::PATH_NOT_FOUND, "Error: file path not found"));
        }
        string _file_name = file_path + ".log";
        file.open(_file_name, ios::out | ios::app);
        if (!file.is_open())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_IO_FAILED, "Error: opening log file"));
        }
        session["task_name"] = file_path;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time), "%Y-%m-%dT%H:%M:%S");

        session["session-timestamp"] = ss.str();
        session["session_log"] = json::array();
        flushed = false;
        is_running = true;

        return Result<void>::Ok();
    }

    Result<void> SessionLogger::log_execution(const ExecutionResult &execution_result)
    {
        if (session.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SESSION_LOGGER_NOT_RUNNING, "Error: session logger not initialized"));
        }
        WatchEvent e = execution_result._event;
        string terminal_msg = execution_result.log;
        vector<string> commands = execution_result.build_commands;
        int success_code = execution_result.id;

        json event;
        event["event_type"] = "modify";
        event["file_path"] = e.path;
        event["file_type"] = e.filetype;
        event["event_mask"] = e.event_mask;

        try
        {
            event["build-command"] = json::array();
            for (auto &cmd : commands)
            {
                event["build-command"].push_back(cmd);
            }
            event["success_code"] = success_code;
            event["terminal_msg"] = terminal_msg;

            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;

            // thread-safe time
            std::tm tm_buf;
            localtime_r(&now_time, &tm_buf); // POSIX, thread-safe
            ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");

            event["timestamp"] = ss.str();

            session["session_log"].push_back(event);
            return Result<void>::Ok();
        }
        catch (const std::bad_alloc &e)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_ALLOC_FAILED, "Error: allocating memory for build-command"));
        }
    }

    Result<void> SessionLogger::stop()
    {
        if (!is_running)
        {
            return Result<void>::Err(FWError::make(ErrorCode::SESSION_LOGGER_NOT_RUNNING, "Error: session logger not running"));
        }

        if (!file.is_open())
        {
            return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED, "Error: couldn't open session logger file"));
        }

        if (!flushed)
        {
            try
            {
                file << session.dump(4) << endl;
            }
            catch (const std::bad_alloc &e)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_ALLOC_FAILED, "Error: allocating memory for session log failed"));
            }
            if (file.fail())
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_IO_FAILED, "Error: writing to session log failed"));
            }
            file.close();
            if (file.fail())
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_IO_FAILED, "Error: closing session log failed"));
            }
            is_running = false;
            flushed = true;
        }
        return Result<void>::Ok();
    }
}