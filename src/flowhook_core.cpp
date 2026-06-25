#include <vector>
#include <string>
#include <filesystem>

#include "include/flowhook_core.h"
#include "include/error/result.h"
#include "include/error/error.h"
#include "include/task_runner.h"
#include "include/macros.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace flowhook {

    Result<FlowHookCore*> FlowHookCore::create(){
        FlowHookCore* core = new FlowHookCore();
        TEST_OVERLOADED(core->init(), FlowHookCore*);
        return Result<FlowHookCore*>::Ok(core);
    }

    FlowHookCore::~FlowHookCore()
    {
        config_manager->flush();
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            delete (*it);
        }
    }

    Result<void> FlowHookCore::set_default_ignored()
    {
        default_ignored_paths = {
            "*.o", "*.a", "*.so", "*.out", "*.exe",
            "*.swp", "*.swo", "*~", ".#*",
            "*.class", "*.pyc", "*.log"
        };
        default_ignored_paths   = {
            ".git"
        };
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::init()
    {
        config_manager = TRY(ConfigManager::create(), void);
        vector<Task> tasks = TRY(config_manager->get_tasks(), void);
        set_default_ignored();
        for(auto task: tasks)
        {
            TaskRunner* tr = TRY(TaskRunner::create(task.name, task.id), void);
            for(auto i: default_ignored_paths)
                tr->add_ignored_path(i);
            for(auto ip: default_ignored_patterns)
                tr->add_ignored_pattern(ip);

            for(auto c: task.commands)
                tr->add_command(c);
            for(auto p: task.paths)
                tr->add_path(p);
            for(auto s: task.on_success)
                tr->add_on_success(s);
            for(auto f: task.on_failure)
                tr->add_on_failure(f);

            for(auto i: task.ignored_paths)
                tr->add_ignored_path(i);
            for(auto ip: task.ignored_patterns)
                tr->add_ignored_pattern(ip);
            if(task.isActive)
            {
                tr->activate();
            }else {
                tr->deactivate();
            }
            task_runners.push_back(tr);
        }
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::create_task(const std::string &task_name, const std::string &task_id)
    {
        std::cout << "create_task called: name='" << task_name << "' id='" << task_id << "'" << std::endl;
        if(task_name.empty())
        {
            std::cout << "task_name is empty" << std::endl;
            return Result<void>::Err(FWError::make(ErrorCode::EMPTY_VALUE, "Error: task name cannot be empty"));
        }
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            if((*it) == nullptr) continue;
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                return Result<void>::Err(FWError::make(ErrorCode::DUPLICATE_ENTRY, "Error: task already exists"));
            }
        }

        if(!fs::exists(task_id) || !fs::is_directory(task_id))
        {
            return Result<void>::Err(FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: working directory not found"));
        }

        if(task_runners.size() >= 100)
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_FULL, "Error: task limit reached"));
        }
        auto result = TRY(TaskRunner::create(task_name, task_id), void);
        task_runners.push_back(result);
        TEST(config_manager->log_task(result->get_task()));
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::delete_task(const std::string &task_id)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = (*it)->get_task_id();
            if(name == task_id)
            {
                Task _task_to_be_deleted = (*it)->get_task();
                TEST(config_manager->delete_task(_task_to_be_deleted));
                task_runners.erase(it);
                return Result<void>::Ok();
            }
        }


        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::activate_task(const std::string &task_name)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string name = (*it)->get_task_name();
            if(name == task_name)
            {
                (*it)->activate();
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::deactivate_task(const std::string &task_id)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                (*it)->deactivate();
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::set_task_path(const std::string &task_id, const std::string &path)
    {
        if(!fs::exists(path))
        {
            return Result<void>::Err(FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: path not found"));
        }
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->add_path(path));
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::delete_task_path(const std::string &task_id, const std::string &path)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->delete_path(path));
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::set_ignored_path(const std::string &task_id, const std::string &path)
    {
        if(!fs::exists(path))
        {
            return Result<void>::Err(FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: path not found"));
        }
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->add_ignored_path(path));
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::set_ignored_pattern(const std::string &task_id, const std::string &pattern)
    {
        if(!fs::exists(pattern))
        {
            return Result<void>::Err(FWError::make(ErrorCode::PATH_NOT_FOUND, "Error: path not found"));
        }
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->add_ignored_pattern(pattern));
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::set_task_command(const std::string &task_id, const std::string &command)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->add_command(command));
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::delete_task_command(const std::string &task_id, const std::string &command)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->delete_command(command));
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::set_task_on_success(const std::string &task_id, const std::string &command)
    {
        if(task_id.empty() || command.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::EMPTY_VALUE, "Error: task id and command cannot be empty"));
        }

        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->add_on_success(command));
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::delete_task_on_success(const std::string &task_id, const std::string &command)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->delete_on_success(command));
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::set_task_on_failure(const std::string &task_id, const std::string &command)
    {
        if(task_id.empty() || command.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::EMPTY_VALUE, "Error: task id and command cannot be empty"));
        }
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->add_on_failure(command));
                config_manager->update_task((*it)->get_task());
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::delete_task_on_failure(const std::string &task_id, const std::string &command)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                TEST((*it)->delete_on_failure(command));
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }



    Result<void> FlowHookCore::start_task(const std::string &task_id)
    {
        FW_LOG("[FLOWHOOK] - starting_task....");
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            FW_LOG("[FLOWHOOK] - looping through tasks to find the correct one ...");
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                FW_LOG("[FLOWHOOK] - task found...");
                if((*it)->is_running())
                {
                    FW_LOG( "[FLOWHOOK] - task is already running.");
                    return Result<void>::Err(FWError::make(ErrorCode::TASK_ALREADY_RUNNING, "Error: task already running"));
                }
                FW_LOG("[FLOWHOOK] - starting the task_runner...");
                (*it)->start();
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_FOUND, "Error: task not found"));
    }

    Result<void> FlowHookCore::stop_task(const std::string &task_id)
    {
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            string id = (*it)->get_task_id();
            if(id == task_id)
            {
                if(!(*it)->is_running())
                {
                    return Result<void>::Err(FWError::make(ErrorCode::TASK_NOT_RUNNING, "Error: task not running"));
                }
                (*it)->stop();
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
            if((*it)->is_running()){
                return Result<void>::Err(FWError::make(ErrorCode::TASK_ALREADY_RUNNING, "Error: task already running"));
            }
            TEST((*it)->start());
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
            TEST((*it)->stop());
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
            if((*it)->is_active())
            {
                TEST((*it)->start());
            }
        }
        return Result<void>::Ok();
    }

    Result<void> FlowHookCore::delete_task(const Task &task)
    {
        return config_manager->delete_task(task);
    }

    std::vector<Task> FlowHookCore::get_tasks() const
    {
        vector<Task> tasks;
        for(auto it = task_runners.begin(); it != task_runners.end(); it++)
        {
            tasks.push_back((*it)->get_task());
        }
        return tasks;
    }
}
