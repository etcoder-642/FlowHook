#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../include/filewatcher.h"

namespace flowhook {
    void print_header();
    std::string receive_input(const std::string &msg);
    void print_list(const std::string &msg, const std::vector<std::string> &list);
    void print_numbered_list(const std::string &msg, const std::vector<std::string> &list);
}