#include <string>
#include <vector>


#include "task_runner.h"
#include "config_manager.h"
#include "error/result.h"
#include "types.h"

namespace flowhook {
    class FlowHookCore {
        private:
            ConfigManager* config_manager;
            std::vector<TaskRunner*> task_runners;
            FlowHookCore() = default;

            bool isValidDir(std::string &path);
            Result<void> init();
        public:
            static Result<FlowHookCore*> create();
            ~FlowHookCore();

            Result<void> create_task(const std::string &task_name, const std::string &working_directory);
            Result<void> delete_task(const std::string &task_name);

            Result<void> start_task(const std::string &task_name);
            Result<void> stop_task(const std::string &task_name);


            Result<void> set_task_path(const std::string &task_name, const std::string &path);
            Result<void> delete_task_path(const std::string &task_name, const std::string &path);

            Result<void> set_task_command(const std::string &task_name, const std::string &command);
            Result<void> delete_task_command(const std::string &task_name, const std::string &command);

            Result<void> set_task_on_success(const std::string &task_name, const std::string &command);
            Result<void> delete_task_on_success(const std::string &task_name, const std::string &command);
            Result<void> set_task_on_failure(const std::string &task_name, const std::string &command);
            Result<void> delete_task_on_failure(const std::string &task_name, const std::string &command);

            Result<void> start_all();
            Result<void> stop_all();

            Result<void> activate_task(const std::string &task_name);
            Result<void> deactivate_task(const std::string &task_name);

            Result<void> start_active();
            Result<void> stop_active();
            Result<void> delete_task(const Task &task);

            std::vector<Task> get_tasks() const;
    };
}
