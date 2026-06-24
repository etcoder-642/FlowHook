#include "../../src/include/flowhook_core.h"
#include "./include/CLI11.hpp"
#include <filesystem>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace flowhook;

int main(int argc, char **argv) {

  auto _fh = FlowHookCore::create();
  if (_fh.isErr()) {
    std::cerr << "Failed to create FlowHook instance: " << _fh.getErrMessage()
              << std::endl;
    return 1;
  }
  FlowHookCore *fh = _fh.unwrap();

  CLI::App app{"FlowHook"};
  app.require_subcommand(0, 1);

  // init subcommand
  std::string task_name = "";
  auto *init =
      app.add_subcommand("init", "initializing a new FlowHook project");

  std::string n_path = "";
  init->add_option("--task", task_name, "Task name to initialize");
  init->add_option("--path", n_path, "Path to the task");

  init->callback([&]() {
    fs::path cwd = fs::current_path();

    if (task_name.empty())
      task_name = cwd.string();
    if (n_path.empty())
      n_path = cwd.string();

    auto r = fh->create_task(task_name, n_path);
    if (r.isErr()) {
      std::cerr << "Failed to create task: " << r.getErrMessage() << std::endl;
      return;
    }
  });

  // add subcommand
  auto *add =
      app.add_subcommand("add", "adding a new task to the FlowHook project");
  std::string add_task_name = "";
  std::string add_n_path = "";
  add->add_option("--task", add_task_name, "Task name to add");
  add->add_option("--path", add_n_path, "Path to the task");

  std::string add_command = "";
  add->add_option("--command", add_command, "Command to run for the task");

  std::string command_on_success = "";
  std::string command_on_failure = "";
  std::string ignored_path = "";
  std::string ignored_pattern = "";

  add->add_option("--on-success", command_on_success,
                  "Command to run on success");
  add->add_option("--on-failure", command_on_failure,
                  "Command to run on failure");
  add->add_option("--ignored-path", ignored_path,
                  "Ignored path");
  add->add_option("--ignored-pattern", ignored_pattern,
                  "Ignored pattern");

  add->callback([&]() {
    fs::path cwd = fs::current_path();

    if (add_task_name.empty())
      add_task_name = cwd.string();
    if (add_n_path.empty() && add_command.empty() &&
        command_on_success.empty() && command_on_failure.empty()) {
      std::cerr << "At least one of --path, --command, --on-success, or "
                   "--on-failure is required"
                << std::endl;
      return;
    }

    if (!add_n_path.empty()) {
      auto r = fh->set_task_path(add_task_name, add_n_path);
      if (r.isErr()) {
        std::cerr << "Failed to set task path: " << r.getErrMessage()
                  << std::endl;
        return;
      }
    }
    if (!add_command.empty()) {
      auto r = fh->set_task_command(add_task_name, add_command);
      if (r.isErr()) {
        std::cerr << "Failed to set task command: " << r.getErrMessage()
                  << std::endl;
        return;
      }
    }
    if (!command_on_success.empty()) {
      auto r = fh->set_task_on_success(add_task_name, command_on_success);
      if (r.isErr()) {
        std::cerr << "Failed to set task on success: " << r.getErrMessage()
                  << std::endl;
        return;
      }
    }
    if (!command_on_failure.empty()) {
      auto r = fh->set_task_on_failure(add_task_name, command_on_failure);
      if (r.isErr()) {
        std::cerr << "Failed to set task on failure: " << r.getErrMessage()
                  << std::endl;
        return;
      }
    }
    if (!ignored_path.empty()) {
      auto r = fh->set_ignored_path(add_task_name, ignored_path);
      if (r.isErr()) {
        std::cerr << "Failed to set ignored path: " << r.getErrMessage()
                  << std::endl;
        return;
      }
    }
    if (!ignored_pattern.empty()) {
      auto r = fh->set_task_on_failure(add_task_name, ignored_pattern);
      if (r.isErr()) {
        std::cerr << "Failed to set ignored pattern: " << r.getErrMessage()
                  << std::endl;
        return;
      }
    }
  });

  // run subcommand

  auto* run = app.add_subcommand("run", "running a task");
  std::string run_task_name = "";
  run->add_option("--task", run_task_name, "Task name to run");

  bool run_all = false;
  run->add_flag("--all", run_all, "Run all tasks");

  run->callback([&]() {
    if(run_task_name.empty())
    {
        run_task_name = fs::current_path().string();
    }

    if (run_all) {
      auto r = fh->start_all();
      if (r.isErr()) {
        std::cerr << "Failed to run all tasks: " << r.getErrMessage()
                  << std::endl;
        return;
      }
      pause();
      return;
    } else if (!run_task_name.empty()) {
      auto r = fh->start_task(run_task_name);
      if (r.isErr()) {
        std::cerr << "Failed to run task: " << r.getErrMessage()
                  << std::endl;
        return;
      }
      pause();
    }
  });

  CLI11_PARSE(app, argc, argv);
  delete fh;
  return 0;
}
