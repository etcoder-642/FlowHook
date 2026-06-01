#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "../include/session_logger.h"

using namespace std;
namespace l_fw
{
    Result<void> SessionLogger::start(string &task_name)
    {
        string _file_name = task_name + ".log";
        file.open(_file_name, ios::out | ios::app);
        if (!file.is_open())
        {
            return Result<void>::Err(ErrorCode::SYSTEM_IO_ERROR, "Error: opening log file");
        }
        session["task_name"] = task_name;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time), "%Y-%m-%dT%H:%M:%S");

        session["timestamp"] = ss.str();
        session["session_log"] = json::array();

        file << session.dump(4) << endl;
        return Result<void>::Ok();
    }

    Result<void> SessionLogger::log_event(_i_event e, int success_code, string terminal_msg, vector<string> commands)
    {
        json event;
        event["event_type"] = "modify";
        event["file_path"] = e.path;
        event["file_type"] = e.filetype;
        event["event_mask"] = e.event_mask;

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
        ss << std::put_time(std::localtime(&now_time), "%Y-%m-%dT%H:%M:%S");

        event["timestamp"] = ss.str();

        session["session_log"].push_back(event);
        file << session.dump(4) << endl;
        return Result<void>::Ok();
    }
}