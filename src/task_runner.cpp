#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <filesystem>

#include "include/task_runner.h"
#include "include/macros.hpp"
using namespace std;

namespace flowhook
{
    Result<TaskRunner*> TaskRunner::create(const string &task_name, const string &working_directory)
    {
        TaskRunner* t = new TaskRunner();
        TEST_OVERLOADED(t->init(task_name, working_directory), TaskRunner*);
        return Result<TaskRunner*>::Ok(t);
    }

    Result<void> TaskRunner::init(const string &task_name, const string &working_directory)
    {
        fw = TRY(FileWatcher::create(), void);
        TEST(fw->init());
        task.name = task_name;
        task.working_directory = working_directory;
        flushed = false;
        task.isRunning = false;
        return Result<void>::Ok();
    }

    TaskRunner::~TaskRunner()
    {
        if(fw)
            fw->stop();
        delete fw;
        sl.stop();
        task.isRunning = false;
    }

    Result<void> TaskRunner::change_task_name(string &task_name)
    {
        if(task.name == task_name)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::TASK_ALREADY_EXISTS, "Error: task name already exists"));
        }
        task.name = task_name;
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::change_working_directory(string &working_directory)
    {
        if(task.working_directory == working_directory)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::PATH_ALREADY_EXISTS, "Error: working directory already exists"));
        }
        task.working_directory = working_directory;
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::add_command(string &command)
    {
        if(command.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::COMMAND_EMPTY, "Error: command is empty"));
        }

        task.commands.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_command(string &command)
    {
        for (auto it = task.commands.begin(); it != task.commands.end(); it++)
        {
            if (*it == command)
            {
                task.commands.erase(it);
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found"));
    }

    Result<void> TaskRunner::add_path(string &path)
    {
        for(auto it = task.paths.begin(); it != task.paths.end(); it++)
        {
            if(*it == path)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::PATH_ALREADY_EXISTS, "Error: path already exists"));
            }
        }
        task.paths.push_back(path);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_path(string &path)
    {
        for (auto it = task.paths.begin(); it != task.paths.end(); it++)
        {
            if (*it == path)
            {
                task.paths.erase(it);
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(FWError::make(ErrorCode::EVENT_NOT_FOUND, "Error: path not found"));
    }

    Result<void> TaskRunner::add_on_success(string &command)
    {
        for(auto it = task.on_success.begin(); it != task.on_success.end(); it++)
        {
            if(*it == command)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::COMMAND_ALREADY_EXISTS, "Error: command already exists"));
            }
        }
        task.on_success.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_on_success(string &command)
    {
        for (auto it = task.on_success.begin(); it != task.on_success.end(); it++)
        {
            if (*it == command)
            {
                task.on_success.erase(it);
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found"));
    }

    Result<void> TaskRunner::add_on_failure(string &command)
    {
        for(auto it = task.on_failure.begin(); it != task.on_failure.end(); it++)
        {
            if(*it == command)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::COMMAND_ALREADY_EXISTS, "Error: command already exists"));
            }
        }
        task.on_failure.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_on_failure(string &command)
    {
        for (auto it = task.on_failure.begin(); it != task.on_failure.end(); it++)
        {
            if (*it == command)
            {
                task.on_failure.erase(it);
                return Result<void>::Ok();
            }
        }

        return Result<void>::Err(FWError::make(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found"));
    }

    Result<void> TaskRunner::execute(const WatchEvent &e)
    {
        if (task.commands.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::COMMAND_EMPTY, "Error: no commands to execute"));
        }        

        for (auto &cmd : task.commands)
        {
            string secure_execution_chain = "cd " + task.working_directory + " && timeout 15s " + cmd + " 2>&1";
            FILE *fp = popen(secure_execution_chain.c_str(), "r");
            if (fp == NULL)
            {
                return Result<void>::Err(FWError::make(ErrorCode::SYS_PIPE_FAILED, "Error: popen failure"));
            }

            char buffer[128];
            string log_output;

            size_t total_bytes = 0;
            const size_t MAX_LOG_SIZE = 64 * 1024;
            while (fgets(buffer, 128, fp) != NULL)
            {
                size_t chunk_size = strlen(buffer);
                total_bytes += chunk_size;
                if (total_bytes < MAX_LOG_SIZE)
                {
                    log_output += buffer;
                    continue;
                }else {
                    log_output += "\n[FLOWHOOK WARNING: Log Truncated. Exceeded 64KB safety limit]";
                    break;
                }
            }
            if(ferror(fp))
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::SYS_IO_FAILED, "Error: reading from pipe failed"));
            }

            int status = pclose(fp);
            int true_exit_code = -1;
            if(WIFEXITED(status)){
                true_exit_code = WEXITSTATUS(status);
            }else if(WIFSIGNALED(status)){
                true_exit_code = -WTERMSIG(status);
            } else {
                true_exit_code = status;
            }

            if (true_exit_code != 0)
            {
                for(auto &cmd_i : task.on_failure)
                {
                    string exec_chain = "cd " + task.working_directory + " && timeout 15s " + cmd_i;
                    system(exec_chain.c_str());
                }
            }
            else 
            {
                for(auto &cmd_i : task.on_success)
                {
                    string exec_chain = "cd " + task.working_directory + " && timeout 15s " + cmd_i;
                    system(exec_chain.c_str());
                }
            }

            ExecutionResult result = {execution_id, true_exit_code, e, log_output, task.commands};
            TEST(sl.log_execution(result));
            execution_id++;
        }

        return Result<void>::Ok();
    }

    Result<void> TaskRunner::add_callback(const WatchCallback &callback)
    {
        callbacks.push_back(callback);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_callback(const WatchCallback &callback)
    {
        for (auto it = callbacks.begin(); it != callbacks.end(); it++)
        {
            if (*it == callback)
            {
                callbacks.erase(it);
                return Result<void>::Ok();
            }
        }
        
        return Result<void>::Err(FWError::make(ErrorCode::CALLBACK_NOT_FOUND, "Error: callback not found"));
    }

    Result<void> TaskRunner::flush()
    {
        if(flushed)
        {
            return Result<void>::Ok();
        }
        if(task.commands.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::COMMAND_NOT_FOUND, "Error: no commands to execute"));
        }

        if(task.paths.empty())
        {
            return Result<void>::Err(FWError::make(ErrorCode::EVENT_NOT_FOUND, "Error: no paths to watch"));
        }

        for (auto &path : task.paths)
        {
            TEST(fw->add_path(path));
        }

        for(auto &cb : callbacks)
        {
            TEST(fw->link_event(IN_CLOSE_WRITE, cb));
        }
        flushed = true;
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::start()
    {
        if(task.isRunning)
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_ALREADY_RUNNING, "Error: task runner already running"));
        }

        string _file_path = task.working_directory + "/" + task.name;
        sl.start(_file_path);


        task.isRunning = true;

        WatchCallback callback = {this, &TaskRunner::execute};
        TEST(add_callback(callback));

        TEST(flush());
        TEST(fw->start(100));
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::stop()
    {
        if(!task.isRunning)
        {
            return Result<void>::Err(ErrorCode::TASK_NOT_RUNNING, "Error: task watcher not running");
        }

        for(auto &cb : callbacks)
        {
            TEST(fw->unlink_event(IN_CLOSE_WRITE, cb));
        }
        TEST(fw->stop());
        TEST(sl.stop());
        task.isRunning = false;
        return Result<void>::Ok();
    }
}

// WRITTEN BY Mnasie