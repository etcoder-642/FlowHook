#include "../../src/include/flowhook_core.h"
#include "../include/CLI11.hpp"
#include "../../src/include/macros.hpp"


namespace fs = std::filesystem;

namespace flowhook_cli {
    void register_run(CLI::App* app, flowhook::FlowHookCore* fh)
    {
        auto* run = app->add_subcommand("run", "running a task");

        // this is a purely ceremonial flag(so that CLI11 won't say there is unrecognized flag)
        // the actual parsing of verbose and setting of the environment variable is done in main.cpp
        run->add_flag("--verbose", FLOWHOOK_VERBOSE, "Enable verbose output");



        static bool run_all = false;
        run->add_flag("--all", run_all, "Run all task instances");

        run->callback([=]() mutable {
          std::string run_task_id = fs::current_path().string();

          if (run_all) {
            auto r = fh->start_all();
            if (r.isErr()) {
              std::cerr << "Failed to run all tasks: " << r.getErrMessage()
                        << std::endl;
              return;
            }
            pause();
            return;
          } else if (!run_task_id.empty()) {
            auto r = fh->start_task(run_task_id);
            if (r.isErr()) {
              std::cerr << "Failed to run task: " << r.getErrMessage()
                        << std::endl;
              return;
            }
            pause();
          }
        });
    }
}
