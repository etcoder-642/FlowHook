#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "include/config_manager.h"
#include "include/macros.hpp"

using namespace std;

namespace flowhook
{
    std::filesystem::path get_config_path()
    {
        const char *home = std::getenv("HOME");
        return std::filesystem::path(home) / ".config" / "flowhook" / "tasks.json";
    }

    Result<void> ensure_config_dir()
    {
        std::error_code ec;
        std::filesystem::create_directories(get_config_path().parent_path(), ec);
        if (ec)
        {
            return Result<void>::Err(FWError::make(ErrorCode::SYS_IO_FAILED, ec.message()));
        }
        return Result<void>::Ok();
    }

    Result<void> ConfigManager::init()
    {
        TEST(ensure_config_dir());
        auto config_path = get_config_path();
        file.open(config_path, ios::in | ios::out | ios::trunc);
        if (!file.is_open())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_IO_FAILED, "Error: opening config file"));
        }
        return Result<void>::Ok();
    }

    Result<json> convert_task_to_json(const Task &task)
    {
        json _json_task = json::object();
        _json_task["task_name"] = task.name;
        _json_task["working_directory"] = task.working_directory;
        _json_task["commands"] = json::array();
        for(auto &cmd : task.commands)
        {
            _json_task["commands"].push_back(cmd);
        }
        _json_task["paths"] = json::array();
        for(auto &path : task.paths)
        {
            _json_task["paths"].push_back(path);
        }
        _json_task["on_success"] = json::array();
        for(auto &cmd : task.on_success)
        {
            _json_task["on_success"].push_back(cmd);
        }
        _json_task["on_failure"] = json::array();
        for(auto &cmd : task.on_failure)
        {
            _json_task["on_failure"].push_back(cmd);
        }
        _json_task["isActive"] = task.isActive;
    }

    Result<void> ConfigManager::log_task(const Task &task)
    {
        json _json_task = TRY(convert_task_to_json(task), void);
        config_obj["tasks"].push_back(_json_task);

        return Result<void>::Ok();
    }

    Result<void> ConfigManager::update_task(const Task &task)
    {
        for(auto it = config_obj["tasks"].begin(); it != config_obj["tasks"].end(); it++)
        {
            string name = it->at("task_name");
            json _json_task = TRY(convert_task_to_json(task), void);
            if(name == task.name)
            {
                *it = _json_task;
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> ConfigManager::purge_config()
    {
        config_obj = json::object();
        return Result<void>::Ok();
    }

    Result<void> ConfigManager::log_task_inbatch(const vector<Task> &tasks)
    {
        for(auto &task : tasks)
        {
            TEST(log_task(task));
        }
        return Result<void>::Ok();
    }

    Result<void> ConfigManager::delete_task(const Task& task)
    {
        for(auto it = config_obj["tasks"].begin(); it != config_obj["tasks"].end(); it++)
        {
            string name = it->at("task_name");
            if(name == task.name)
            {
                config_obj["tasks"].erase(it);
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<vector<Task>> ConfigManager::get_tasks()
    {
        vector<Task> tasks;
        for(auto it = config_obj["tasks"].begin(); it != config_obj["tasks"].end(); it++)
        {
            Task _task;
            _task.name = it->at("task_name");
            _task.working_directory = it->at("working_directory");
            _task.commands = it->at("commands");
            _task.paths = it->at("paths");
            _task.on_success = it->at("on_success");
            _task.on_failure = it->at("on_failure");
            _task.isActive = it->at("isActive");
            tasks.push_back(_task);
        }
        return Result<vector<Task>>::Ok(tasks);
    }

    Result<void> ConfigManager::flush()
    {
        if(isflushed)
        {
            return Result<void>::Ok();
        }
        config_obj = json::object();
        file << config_obj.dump(4) << endl;
        if (file.fail())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_IO_FAILED, "Error: writing to config file failed"));
        }
        isflushed = true;
        return Result<void>::Ok();
    }
}