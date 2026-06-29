cmake_minimum_required(VERSION 3.16)

project(FlowHook VERSION 0.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(src/version.h.in version.h)
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

add_executable(test_session_logger tests/unit/test_session_logger.cpp)
target_link_libraries(test_session_logger PRIVATE flowhook_lib)
target_include_directories(test_session_logger PRIVATE tests)

add_executable(test_config_manager tests/unit/test_config_manager.cpp)
target_link_libraries(test_config_manager PRIVATE flowhook_lib)
target_include_directories(test_config_manager PRIVATE tests)
target_compile_definitions(test_config_manager PRIVATE FLOWHOOK_TESTING)

add_executable(test_flowhook_core tests/integration/test_flowhook_core.cpp)
target_link_libraries(test_flowhook_core PRIVATE flowhook_lib)
target_include_directories(test_flowhook_core PRIVATE tests)

add_executable(flowhook_cli cli/src/main.cpp
                            cli/src/commands/add.cpp
                            cli/src/commands/run.cpp
                            cli/src/commands/init.cpp
                            cli/src/commands/list.cpp)
target_link_libraries(flowhook_cli PRIVATE flowhook_lib)
target_include_directories(flowhook_cli PRIVATE cli/src)

enable_testing()
add_test(NAME flowhook_filewatcher_tests COMMAND test_filewatcher)
add_test(NAME flowhook_task_runner_tests COMMAND test_task_runner)
add_test(NAME flowhook_session_logger_tests COMMAND test_session_logger)
add_test(NAME flowhook_config_manager_tests COMMAND test_config_manager)
add_test(NAME flowhook_core_tests COMMAND test_flowhook_core)
add_test(NAME flowhook_cli_tests COMMAND flowhook_cli)
