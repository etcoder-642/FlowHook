#include <fstream>

#include "error/result.h"
#include "json.hpp"
#include "types.h"



namespace flowhook
{
    class ConfigManager {
        #ifdef FLOWHOOK_TESTING
            friend class ConfigManagerTest;
        #endif
        private:
            nlohmann::json config_obj = nlohmann::json::object();

            bool isflushed;
            Result<nlohmann::json> convert_task_to_json(const Task &task);
            Result<Task> convert_json_to_task(const nlohmann::json &json_task);

            ConfigManager() = default;
            Result<void> load(std::fstream &file);
        public:
            static Result<ConfigManager*> create();
            ~ConfigManager();
            nlohmann::json getjson() const { return config_obj; }
            bool is_flushed() const { return isflushed; }

            Result<void> log_task(const Task &task);
            Result<void> update_task(const Task &task);
            Result<void> purge_config();
            Result<void> log_task_inbatch(const std::vector<Task> &tasks);
            Result<void> delete_task(const Task &task);
            Result<std::vector<Task>> get_tasks();
            Result<void> flush();
    };
}
