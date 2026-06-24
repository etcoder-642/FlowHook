#include <sys/inotify.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <filesystem>
#include <chrono>

#include "include/task_runner.h"
#include "error/error.h"
#include "error/result.h"
#include "include/macros.hpp"

namespace fs = std::filesystem;
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
        task.name = task_name;
        task.working_directory = working_directory;
        flushed = false;
        task.isRunning = false;

        sl = new SessionLogger();
        if (!sl)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::SYS_ALLOC_FAILED, "Error: allocating memory for session logger"));
        }
        TEST(sl->start(working_directory));

        last_executed = std::chrono::steady_clock::now() - std::chrono::milliseconds(300);
        return Result<void>::Ok();
    }

    TaskRunner::~TaskRunner()
    {
        if(fw)
            fw->stop();
        delete fw;
        sl->stop();
        delete sl;
        task.isRunning = false;
    }

    Result<void> TaskRunner::set_depth(int num)
    {
        if(num > 6)
        {
            return Result<void>::Err(
                FWError::make(ErrorCode::INVALID_DEPTH, "Error: invalid depth set - depth too much")
            );
        } else if( num < 1)
        {
            return Result<void>::Err(
                FWError::make(ErrorCode::INVALID_DEPTH, "Error: invalid depth set - depth set too low")
            );
        }

        task.watching_depth = num;
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::change_task_name(const string &task_name)
    {
        // check if task name is empty
        if(task_name.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: task name is empty"
            ));
        }
        // check if task name already exists
        if(task.name == task_name)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::TASK_ALREADY_EXISTS, "Error: task name already exists"));
        }
        task.name = task_name;
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::change_working_directory(const string &working_directory)
    {
        // check if working directory is empty
        if(working_directory.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: working directory is empty"
            ));
        }
        // check if working directory is a valid path
        if(!fs::exists(working_directory))
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::PATH_NOT_FOUND, "Error: working directory not found"
            ));
        }
        // check if working directory already exists
        if(task.working_directory == working_directory)
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::PATH_ALREADY_EXISTS, "Error: working directory already exists"));
        }
        task.working_directory = working_directory;
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::add_ignored_path(const string &path)
    {
        if(path.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: command is empty"));
        }
        for(auto &p : task.ignored_paths)
        {
            if(p == path)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::PATH_ALREADY_EXISTS, "Error: path already exists"));
            }
        }
        task.ignored_paths.push_back(path);
        fw->add_ignored_path(path);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::add_ignored_pattern(const string &pattern)
    {
        if(pattern.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: command is empty"));
        }
        for(auto &p : task.ignored_patterns)
        {
            if(p == pattern)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::PATH_ALREADY_EXISTS, "Error: path already exists"));
            }
        }
        task.ignored_patterns.push_back(pattern);
        fw->add_ignored_pattern(pattern);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::add_command(const string &command)
    {
        // check if command is empty
        if(command.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: command is empty"));
        }

        // check if command already exists
        for(auto &cmd : task.commands)
        {
            if(cmd == command)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::COMMAND_ALREADY_EXISTS, "Error: command already exists"));
            }
        }

        task.commands.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_command(const string &command)
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

    Result<void> TaskRunner::add_path(const string &path)
    {
        TEST(fw->add_path(path, task.watching_depth));
        task.paths.push_back(path);
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::delete_path(const string &path)
    {
        for (auto it = task.paths.begin(); it != task.paths.end(); it++)
        {
            if (*it == path)
            {
                task.paths.erase(it);
                return Result<void>::Ok();
            }
        }
        TEST(fw->remove_path(path));
        return Result<void>::Err(FWError::make(ErrorCode::EVENT_NOT_FOUND, "Error: path not found"));
    }

    Result<void> TaskRunner::add_on_success(const string &command)
    {
        // check if command is empty
        if(command.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: command is empty"));
        }

        // check if command already exists
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

    Result<void> TaskRunner::delete_on_success(const string &command)
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

    Result<void> TaskRunner::add_on_failure(const string &command)
    {
        // check if command is empty
        if(command.empty())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: command is empty"));
        }
        // check if command already exists
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

    Result<void> TaskRunner::delete_on_failure(const string &command)
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
        auto now = chrono::steady_clock::now();
        if (now - last_executed < cooldown_ms)
            return Result<void>::Ok(); // silently skip

        FW_LOG("[DEBUG] execute called");
        // check if event is null
        if(e.isNull())
        {
            FW_LOG("[FLOWHOOK] - event is null.");
            return Result<void>::Err(FWError::make(ErrorCode::EVENT_NOT_FOUND, "Error: event is null"));
        }

        // check if task commands are empty
        if (task.commands.empty())
        {
            FW_LOG("[FLOWHOOK] - command is empty.");
            return Result<void>::Err(FWError::make(ErrorCode::COMMAND_EMPTY, "Error: no commands to execute"));
        }

        for (auto &cmd : task.commands)
        {
            FW_LOG("[FLOWHOOK] - executing command through pipes ....");
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
                    FW_LOG("[FLOWHOOK] - failure command executing...");
                    string exec_chain = "cd " + task.working_directory + " && timeout 15s " + cmd_i;
                    system(exec_chain.c_str());
                }
            }
            else
            {
                for(auto &cmd_i : task.on_success)
                {
                    FW_LOG("[FLOWHOOK] - success command executing...");
                    string exec_chain = "cd " + task.working_directory + " && timeout 15s " + cmd_i;
                    system(exec_chain.c_str());
                }
            }

            ExecutionResult result = {execution_id, true_exit_code, e, log_output, task.commands};
            TEST(sl->log_execution(result));
            execution_id++;
        }
        last_executed = chrono::steady_clock::now();
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::add_callback(const WatchCallback &callback)
    {
        // check if callback is empty
        if(callback.isNull())
        {
            return Result<void>::Err(FWError::make(
                ErrorCode::EMPTY_VALUE, "Error: callback is empty"));
        }
        // check if callback already exists
        for(auto &cb : callbacks)
        {
            if(cb == callback)
            {
                return Result<void>::Err(FWError::make(
                    ErrorCode::CALLBACK_ALREADY_EXISTS, "Error: callback already exists"));
            }
        }
        //
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

    Result<void> TaskRunner::start()
    {
        FW_LOG("[FLOWHOOK] - task_runner started...");
        if(task.isRunning)
        {
            return Result<void>::Err(FWError::make(ErrorCode::TASK_ALREADY_RUNNING, "Error: task runner already running"));
        }

        string _file_path = task.working_directory + "/" + task.name;
        FW_LOG("[FLOWHOOK] - adding path " << _file_path << " to session logger...");
        sl->start(_file_path);


        task.isRunning = true;

        WatchCallback callback = {this, &TaskRunner::execute};
        TEST(add_callback(callback));
        FW_LOG("[FLOWHOOK]  - linking callback with events...");
        for(auto &cb : callbacks)
        {
            TEST(fw->link_event(IN_CLOSE_WRITE, cb));
            TEST(fw->link_event(IN_MODIFY, cb));
            TEST(fw->link_event(IN_MOVED_FROM, cb));
        }


        FW_LOG("[FLOWHOOK] - starting filewatcher...");
        TEST(fw->start(100));
        return Result<void>::Ok();
    }

    Result<void> TaskRunner::stop()
    {
        if(!task.isRunning)
        {
            return Result<void>::Err(ErrorCode::TASK_NOT_RUNNING, "Error: task watcher not running");
        }

        int count = 0;
        for(auto &cb : callbacks)
        {
            TEST(fw->unlink_event(IN_CLOSE_WRITE, cb));
            count++;
        }
        TEST(fw->stop());
        TEST(sl->stop());
        task.isRunning = false;
        return Result<void>::Ok();
    }
}

// WRITTEN BY Mnasie
