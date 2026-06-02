#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "filewatcher.h"
#include "display.h"
#include "json.hpp"
#include "error/error.h"
#include "error/result.h"

namespace l_fw
{
    using json = nlohmann::json;
    class SessionLogger {
        private:
            std::fstream file;
            json session;

            bool is_running;
            bool flushed;
        public:
            SessionLogger();
            ~SessionLogger();

            Result<void> start(std::string &task_name);
            Result<void> log_event(_i_event e, int success_code, std::string terminal_msg, std::vector<std::string> commands);
            Result<void> stop();
    };
}