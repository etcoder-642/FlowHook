#include <vector>
#include <string>
#include <filesystem>

#include "include/flowhook_core.h"
#include "include/error/result.h"
#include "include/task_runner.h"
#include "include/macros.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace flowhook {


    Result<void> FlowHookCore::create_task(std::string &task_name, std::string &working_directory)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = it->get_task_name();
            if(name == task_name)
            {
                return Result<void>::Err(FWError::make(ErrorCode::DUPLICATE_ENTRY, "Error: task already exists"));
            }
        }

        if(fs::is_directory(working_directory) == false)
        {
            return Result<void>::Err(FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: working directory not found"));
        }

        if(task_runners.size() >= 100)
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_FULL, "Error: task limit reached"));
        }
        TaskRunner* t;
        t->init(task_name, working_directory);
        task_runners.push_back(*t);
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::delete_task(std::string &task_name)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = it->get_task_name();
            if(name == task_name)
            {
                task_runners.erase(it);
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::activate_task(std::string &task_name)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = it->get_task_name();
            if(name == task_name)
            {
                it->activate();
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }
    
    Result<void> FlowHookCore::deactivate_task(std::string &task_name)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = it->get_task_name();
            if(name == task_name)
            {
                it->deactivate();
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::start_task(std::string &task_name)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = it->get_task_name();
            if(name == task_name)
            {
                it->start();
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::stop_task(std::string &task_name)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = it->get_task_name();
            if(name == task_name)
            {
                it->stop();
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::start_all()
    {
        if(task_runners.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: no tasks to start"));
        }

        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            auto execute = it->start();
            if(execute.isErr())
            {
                return Result<void>::Err(execute.unwrapErr());
            }
        }
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::stop_all()
    {
        if(task_runners.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: no tasks to stop"));
        }

        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            TEST(it->stop());
        }
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::start_active()
    {
        if(task_runners.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: no tasks to start"));
        }

        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            if(it->is_active())
            {
                TEST(it->start());
            }
        }
    }

    Result<void> FlowHookCore::create_config_file()
    {
        return config_manager.init();
    }

    Result<void> FlowHookCore::delete_config_file()
    {
        return config_manager.purge_config();
    }

    Result<void> FlowHookCore::log_task(const Task &task)
    {
        return config_manager.log_task(task);
    }

    Result<void> FlowHookCore::update_task(const Task &task)
    {
        return config_manager.update_task(task);
    }

    Result<void> FlowHookCore::log_task_inbatch(const std::vector<Task> &tasks)
    {
        return config_manager.log_task_inbatch(tasks);
    }

    Result<void> FlowHookCore::delete_task(const Task &task)
    {
        return config_manager.delete_task(task);
    }

    std::vector<Task> FlowHookCore::get_tasks() const
    {
        vector<Task> tasks;
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            tasks.push_back(it->get_task());
        }
        return tasks;
    }
}