#include <string>
#include <vector>


#include "task_runner.h"
#include "display.h"
#include "session_logger.h"
#include "config_manager.h"
#include "error/result.h"
#include "types.h"

namespace flowhook {
    class FlowHookCore {
        private:
            ConfigManager* config_manager;
            std::vector<TaskRunner*> task_runners;
            FlowHookCore() = default;
        public:
            static Result<FlowHookCore*> create();
            ~FlowHookCore();

            Result<void> init();

            Result<void> create_task(std::string &task_name, std::string &working_directory);
            Result<void> delete_task(std::string &task_name);

            Result<void> start_task(std::string &task_name);
            Result<void> stop_task(std::string &task_name);


            Result<void> set_task_path(std::string &task_name, std::string &path);
            Result<void> delete_task_path(std::string &task_name, std::string &path);

            Result<void> set_task_command(std::string &task_name, std::string &command);
            Result<void> delete_task_command(std::string &task_name, std::string &command);

            Result<void> set_task_on_success(std::string &task_name, std::string &command);
            Result<void> delete_task_on_success(std::string &task_name, std::string &command);
            Result<void> set_task_on_failure(std::string &task_name, std::string &command);
            Result<void> delete_task_on_failure(std::string &task_name, std::string &command);

            Result<void> start_all();
            Result<void> stop_all();

            Result<void> activate_task(std::string &task_name);
            Result<void> deactivate_task(std::string &task_name);

            Result<void> start_active();
            Result<void> stop_active();
            Result<void> delete_task(const Task &task);

            std::vector<Task> get_tasks() const;
    };
}