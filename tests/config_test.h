#pragma once

#include "../src/include/config_manager.h"
#include "../src/include/json.hpp"

namespace flowhook
{
    // ---------------------------------------------------------------------------
    // ConfigManagerTest
    // ---------------------------------------------------------------------------

    class ConfigManagerTest
    {
    public:
        static nlohmann::json &get_config(ConfigManager *cm) { return cm->config_obj; }
    };
}
