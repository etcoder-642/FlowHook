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
add_executable(tests 
     tests/unit/test_filewatcher.cpp
)
target_link_libraries(tests PRIVATE flowhook_lib)
target_include_directories(tests PUBLIC tests)

enable_testing()
add_test(NAME flowhook_tests COMMAND tests)