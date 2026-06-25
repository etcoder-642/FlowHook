#include "../../src/include/flowhook_core.h"
#include "./include/CLI11.hpp"
#include <iostream>
#include <unistd.h>

#include "../../src/include/macros.hpp"
#include "commands/add.cpp"
#include "commands/init.cpp"
#include "commands/run.cpp"
#include "commands/list.cpp"

using namespace flowhook;
bool FLOWHOOK_VERBOSE = false;

int main(int argc, char **argv) {

  for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--verbose")
            FLOWHOOK_VERBOSE = true;

  auto _fh = FlowHookCore::create();
  if (_fh.isErr()) {
    std::cerr << "Failed to create FlowHook instance: " << _fh.getErrMessage()
              << std::endl;
    return 1;
  }
  FlowHookCore *fh = _fh.unwrap();

  CLI::App app{"FlowHook"};
  app.require_subcommand(0, 1);

  flowhook_cli::register_init(&app, fh);
  flowhook_cli::register_add(&app, fh);
  flowhook_cli::register_run(&app, fh);
  flowhook_cli::register_list(&app, fh);

  CLI11_PARSE(app, argc, argv);
  delete fh;
  return 0;
}
