#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "filewatcher.h"
#include "json.hpp"
#include "error/result.h"
#include "types.h"

namespace flowhook
{
    using json = nlohmann::json;
    class SessionLogger {
        private:
            std::fstream file;
            json session = json::object();

            bool is_running;
            bool flushed;
        public:
            SessionLogger();
            ~SessionLogger();

            Result<void> start(const std::string &file_path);
            Result<void> log_execution(const ExecutionResult &execution_result);
            Result<void> stop();
    };
}