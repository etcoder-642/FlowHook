#include <fstream>
#include <vector>
#include <string>

#include "error/error.h"
#include "error/result.h"
#include "json.hpp"
#include "task_runner.h"


namespace flowhook 
{
    class ConfigManager {
        private:
            fstream file;
            json config_obj = json::object();

            Result<json> convert_task_to_json(const Task &task);
        public:
            ConfigManager();
            ~ConfigManager();

            Result<void> init();
            Result<void> log_task(const Task &task);
            Result<void> update_task(const Task &task);
            Result<void> purge_config();
            Result<void> log_task_inbatch(const std::vector<Task> &tasks);
            Result<void> delete_task(const Task &task);
            Result<std::vector<Task>> get_tasks();
    };
}