#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "filewatcher.h"
#include "session_logger.h"
#include "types.h"

namespace flowhook
{
    class TaskRunner
    {
    private:
        FileWatcher *fw = nullptr;
        SessionLogger sl;
        Task task;
        std::vector<WatchCallback> callbacks;

        bool is_running;
        bool flushed;
        bool is_init = false;
        int execution_id = 0;

        TaskRunner() = default;
    public:
        Result<void> init(const std::string &task_name, const std::string &working_directory);
        ~TaskRunner();

        std::string get_task_name() const { return task.name; }
        std::string get_working_directory() const { return task.working_directory; }
        Task get_task() const { return task; }

        bool is_active() const { return task.isActive; }
        void activate() { task.isActive = true; }
        void deactivate() { task.isActive = false; }

        Result<void> change_task_name(std::string &task_name);
        Result<void> change_working_directory(std::string &working_directory);
        Result<void> add_command(std::string &command);
        Result<void> delete_command(std::string &command);

        Result<void> add_on_success(std::string &command);
        Result<void> delete_on_success(std::string &command);

        Result<void> add_on_failure(std::string &command);
        Result<void> delete_on_failure(std::string &command);

        Result<void> add_path(std::string &path);
        Result<void> delete_path(std::string &path);

        Result<void> add_callback(const WatchCallback &callback);
        Result<void> delete_callback(const WatchCallback &callback);

        Result<void> flush();

        Result<void> execute(const WatchEvent &e);
        Result<void> start();
        Result<void> stop();
    };
}