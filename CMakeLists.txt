cmake_minimum_required(VERSION 3.16)

project(FlowHook)
set(CMAKE_CXX_STANDARD 20)

find_package(ftxui CONFIG REQUIRED)

add_executable(flowhook src/main.cpp src/filewatcher.cpp src/display.cpp src/session_logger.cpp src/task_watcher.cpp)
target_link_libraries(flowhook PRIVATE ftxui::screen ftxui::dom ftxui::component)