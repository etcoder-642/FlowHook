cmake_minimum_required(VERSION 3.16)

project(FlowHook)
set(CMAKE_CXX_STANDARD 20)

find_package(ftxui CONFIG REQUIRED)

# core library
add_library(flowhook_lib
    src/filewatcher.cpp
    src/session_logger.cpp
    src/task_runner.cpp
    src/flowhook_core.cpp
    src/config_manager.cpp
)

target_link_libraries(flowhook_lib PRIVATE ftxui::screen ftxui::dom ftxui::component)
target_include_directories(flowhook_lib PUBLIC src/include)

# main executables
add_executable(flowhook src/main.cpp)
target_link_libraries(flowhook PRIVATE flowhook_lib)


# tests
add_executable(test_filewatcher tests/unit/test_filewatcher.cpp)
target_link_libraries(test_filewatcher PRIVATE flowhook_lib)
target_include_directories(test_filewatcher PRIVATE tests)

add_executable(test_task_runner tests/unit/test_task_runner.cpp)
target_link_libraries(test_task_runner PRIVATE flowhook_lib)
target_include_directories(test_task_runner PRIVATE tests)

enable_testing()
add_test(NAME flowhook_filewatcher_tests COMMAND test_filewatcher)
add_test(NAME flowhook_task_runner_tests COMMAND test_task_runner)