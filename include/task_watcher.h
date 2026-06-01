#pragma once

#include <iostream>
#include <vector>
#include <string>


#include "filewatcher.h"
#include "display.h"
#include "session_logger.h"

namespace l_fw
{
    struct Task {
        std::string name;
        std::vector<std::string> commands;
        std::vector<std::string> paths;

        std::string on_success;
        std::string on_failure;
    };

    class TaskWatcher {
        private:
            FileWatcher fw;
            SessionLogger sl;
            Task task;
            
        public:
            TaskWatcher(const std::string& task_name){
                task.name = task_name;
            }
            ~TaskWatcher();
            Result<void> alter_task_name(std::string &task_name);

            Result<void> add_command(std::string &command);
            Result<void> delete_command(std::string &command);

            Result<void> add_path(std::string &path);
            Result<void> delete_path(std::string &path);
            
            Result<void> execute(const _i_event &e);
            Result<void> start();
            Result<void> stop();
    };
}