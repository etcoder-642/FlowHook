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
            std::fstream file;
            json config_obj = json::object();

            Result<json> convert_task_to_json(const Task &task);
            Result<Task> convert_json_to_task(const json &json_task);
            bool isflushed;
            ConfigManager() = default;
        public:
            static Result<ConfigManager*> create();
            ~ConfigManager();
            bool is_flushed() const { return isflushed; }

            Result<void> init();
            Result<std::vector<Task>> load();
            Result<void> log_task(const Task &task);
            Result<void> update_task(const Task &task);
            Result<void> purge_config();
            Result<void> log_task_inbatch(const std::vector<Task> &tasks);
            Result<void> delete_task(const Task &task);
            Result<std::vector<Task>> get_tasks();
            Result<void> flush();
    };
}