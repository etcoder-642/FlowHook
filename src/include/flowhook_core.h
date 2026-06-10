#include <string>
#include <vector>


#include "task_runner.h"
#include "display.h"
#include "session_logger.h"
#include "config_manager.h"
#include "error/result.h"

namespace flowhook {
    class FlowHookCore {
        private:
            std::vector<TaskRunner> task_runners;
        public:
            FlowHookCore();
            ~FlowHookCore();

            Result<void> create_task(std::string &task_name, std::string &working_directory);
            Result<void> delete_task(std::string &task_name);

            Result<void> start_task(std::string &task_name);
            Result<void> stop_task(std::string &task_name);

            Result<void> start_all();
            Result<void> stop_all();

            Result<void> activate_task(std::string &task_name);
            Result<void> deactivate_task(std::string &task_name);

            Result<void> start_active();
            Result<void> stop_active();
    };
}