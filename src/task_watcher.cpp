#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

#include "../include/task_watcher.h"

using namespace std;
namespace l_fw
{
    TaskWatcher::TaskWatcher(const string &task_name, const string &working_directory)
    {
        task.name = task_name;
        task.working_directory = working_directory;
    }

    TaskWatcher::~TaskWatcher()
    {
        fw.stop();
        sl.stop();
        is_running = false;
    }

    Result<void> TaskWatcher::change_task_name(string &task_name)
    {
        task.name = task_name;
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::change_working_directory(string &working_directory)
    {
        task.working_directory = working_directory;
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::add_command(string &command)
    {
        task.commands.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::delete_command(string &command)
    {
        for (auto it = task.commands.begin(); it != task.commands.end(); it++)
        {
            if (*it == command)
            {
                task.commands.erase(it);
                return Result<void>::Ok();
            }
        }

        cerr << "Error: command not found" << endl;
        return Result<void>::Err(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found");
    }

    Result<void> TaskWatcher::add_path(string &path)
    {
        task.paths.push_back(path);
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::delete_path(string &path)
    {
        for (auto it = task.paths.begin(); it != task.paths.end(); it++)
        {
            if (*it == path)
            {
                task.paths.erase(it);
                return Result<void>::Ok();
            }
        }
        return Result<void>::Err(ErrorCode::EVENT_NOT_FOUND, "Error: path not found");
    }

    Result<void> TaskWatcher::add_on_success(string &command)
    {
        task.on_success.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::delete_on_success(string &command)
    {
        for (auto it = task.on_success.begin(); it != task.on_success.end(); it++)
        {
            if (*it == command)
            {
                task.on_success.erase(it);
                return Result<void>::Ok();
            }
        }

        cerr << "Error: command not found" << endl;
        return Result<void>::Err(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found");
    }

    Result<void> TaskWatcher::add_on_failure(string &command)
    {
        task.on_failure.push_back(command);
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::delete_on_failure(string &command)
    {
        for (auto it = task.on_failure.begin(); it != task.on_failure.end(); it++)
        {
            if (*it == command)
            {
                task.on_failure.erase(it);
                return Result<void>::Ok();
            }
        }

        cerr << "Error: command not found" << endl;
        return Result<void>::Err(ErrorCode::COMMAND_NOT_FOUND, "Error: command not found");
    }

    Result<void> TaskWatcher::execute(const _i_event &e)
    {
        if (task.commands.empty())
        {
            cerr << "Error: no commands to execute" << endl;
            return Result<void>::Err(ErrorCode::COMMAND_NOT_FOUND, "Error: no commands to execute");
        }        

        for (auto &cmd : task.commands)
        {
            string secure_execution_chain = "cd " + task.working_directory + " && timeout 15s " + cmd + " 2>&1";
            FILE *fp = popen(secure_execution_chain.c_str(), "r");
            if (fp == NULL)
            {
                cerr << "Error: popen failure" << endl;
                return Result<void>::Err(ErrorCode::PIPE_ERR, "Error: popen failure");
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

            int status = pclose(fp);
            int true_exit_code = -1;
            if(WIFEXITED(status)){
                true_exit_code = WEXITSTATUS(status);
            }else {
                true_exit_code = status;
            }

            if (true_exit_code != 0)
            {
                for(auto &cmd : task.on_failure)
                {
                    string exec_chain = "cd " + task.working_directory + " && timeout 15s " + cmd;
                    system(exec_chain.c_str());
                    cout << "[FLOWHOOK] Command " << cmd << " executed with exit code " << true_exit_code << endl;
                }
            }
            else 
            {
                for(auto &cmd : task.on_success)
                {
                    string exec_chain = "cd " + task.working_directory + " && timeout 15s " + cmd;
                    system(exec_chain.c_str());
                    cout << "[FLOWHOOK] Command " << cmd << " executed with exit code " << true_exit_code << endl;
                }
            }

            sl.log_event(e, true_exit_code, log_output, task.commands);
        }

        return Result<void>::Ok();
    }


    Result<void> TaskWatcher::start(void(*callback)(const _i_event &e))
    {
        if(is_running)
        {
            cout << "[FLOWHOOK]:ERROR Task watcher already running" << endl;
            return Result<void>::Err(ErrorCode::ALREADY_RUNNING, "Error: task watcher already running");
        }

        if(task.commands.empty())
        {
            cout << "[FLOWHOOK]:ERROR No commands to execute" << endl;
            return Result<void>::Err(ErrorCode::COMMAND_NOT_FOUND, "Error: no commands to execute");
        }

        if(task.paths.empty())
        {
            cout << "[FLOWHOOK]:ERROR No paths to watch" << endl;
            return Result<void>::Err(ErrorCode::EVENT_NOT_FOUND, "Error: no paths to watch");
        }

        cout << "[FLOWHOOK] Starting task: " << task.name << endl;
        string _file_path = task.working_directory + "/" + task.name;
        sl.start(_file_path);

        for (auto &path : task.paths)
        {
            cout << "[FLOWHOOK] Setting up watch on " << path << endl;
            fw.add_path(path);
        }

        is_running = true;
        cout << "[FLOWHOOK] Linking callback to event... " << endl;
        fw.link_event(IN_CLOSE_WRITE, callback);
        cout << "[FLOWHOOK] Starting event loop... " << endl;
        fw.start(100);
        cout << "[FLOWHOOK] Task started successfully... " << endl;
        return Result<void>::Ok();
    }

    Result<void> TaskWatcher::stop(void(*callback)(const _i_event &e))
    {
        if(!is_running)
        {
            return Result<void>::Err(ErrorCode::NOT_RUNNING, "Error: task watcher not running");
        }

        fw.unlink_event(IN_CLOSE_WRITE, callback);
        fw.stop();
        sl.stop();
        is_running = false;
        return Result<void>::Ok();
    }
}

// WRITTEN BY Mnasie