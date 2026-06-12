#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "types.h"
#include "filewatcher.h"
#include "session_logger.h"

namespace flowhook
{
    class FileWatcher;
    class TaskRunner
    {
    private:
        FileWatcher *fw = nullptr;
        SessionLogger *sl;
        Task task;
        std::vector<WatchCallback> callbacks;

        bool flushed;
        bool is_init = false;
        int execution_id = 0;

        TaskRunner() = default;
        Result<void> init(const std::string &task_name, const std::string &working_directory);
    public:
        static Result<TaskRunner*> create(const std::string &task_name, const std::string &working_directory);
        ~TaskRunner();

        std::string get_task_name() const { return task.name; }
        std::string get_working_directory() const { return task.working_directory; }
        Task get_task() const { return task; }

        bool is_active() const { return task.isActive; }
        void activate() { task.isActive = true; }
        void deactivate() { task.isActive = false; }
        bool is_running() const { return task.isRunning; }

        Result<void> change_task_name(const std::string &task_name);
        Result<void> change_working_directory(const std::string &working_directory);
        Result<void> add_command(const std::string &command);
        Result<void> delete_command(const std::string &command);

        Result<void> add_on_success(const std::string &command);
        Result<void> delete_on_success(const std::string &command);

        Result<void> add_on_failure(const std::string &command);
        Result<void> delete_on_failure(const std::string &command);

        Result<void> add_path(const std::string &path);
        Result<void> delete_path(const std::string &path);

        Result<void> add_callback(const WatchCallback &callback);
        Result<void> delete_callback(const WatchCallback &callback);

        Result<void> execute(const WatchEvent &e);
        Result<void> start();
        Result<void> stop();
    };
}