#include <vector>
#include <string>
#include <iostream>

#include <stdio.h>
#include <fstream>

#include "include/filewatcher.h"
#include "include/display.h"
#include "include/task_runner.h"
#include "include/session_logger.h"
#include "include/error/error.h"
#include "include/error/result.h"

using namespace std;
using namespace flowhook;

/*
   THINGS TO WORK ON tomorrow:
   1. fix todos listed in flowhook_core.cpp | DONE
   2. error handling
      - standardize ErrorCode enum (group by category, fix duplicate COMMAND_NOT_FOUND) | DONE
      - make error struct more robust | DONE
      - add stack traceability to error struct | DONE
      - ensure every method handles errors | DONE
      - add TRY macro | DONE
   3. add config_manager layer
   4. add some test cases using utest.h
   5. split CMakeLists.txt
      - separate library target from CLI target
      - wire tests target
   6. start working on the CLI layer
   7. add IN_MOVED_TO and IN_MOVED_FROM events
*/

int main()
{
    TaskRunner* t;
    t->init("test", "/");
    print_header();
    cout << "FOR TESTING PURPOSES ONLY" << endl;
    string temp_task_name = "fitTrack";
    string temp_working_directory = "/home/mnasie/coding/C++/fitTrack";
    string temp_command = "g++ main.cpp -o main";
    string temp_success_cmd = "ffplay -nodisp -autoexit -loglevel quiet success.mp3";
    string temp_failure_cmd = "ffplay -nodisp -autoexit -loglevel quiet fahh_meme.mp3";

    t->change_task_name(temp_task_name);
    t->change_working_directory(temp_working_directory);
    t->add_path(temp_working_directory);
    t->add_command(temp_command);
    t->add_on_success(temp_success_cmd);
    t->add_on_failure(temp_failure_cmd);

    // string project_name = receive_input("Input the watch instance name: ");
    // string file_path = receive_input("Input the directory path to watch: ");
    // t->change_task_name(project_name);
    // t->change_working_directory(file_path);
    // t->add_path(file_path);

    string usr_input;

    while (usr_input != "exit")
    {
        usr_input = receive_input("Input Value(help -- to see all commands): ");
        if (usr_input == "add-command")
        {
            string path = receive_input("Input Command: ");
            t->add_command(path);
        }
        else if (usr_input == "remove-command")
        {
            string path = receive_input("Input Command: ");
            t->delete_command(path);
        }
        else if (usr_input == "add-path")
        {
            string path = receive_input("Input file/directory path: ");
            t->add_path(path);
        }
        else if (usr_input == "remove-path")
        {
            string path = receive_input("Input file/directory path: ");
            t->delete_path(path);
        }
        else if (usr_input == "add-on-success")
        {
            string command = receive_input("Input command: ");
            t->add_on_success(command);
        }
        else if (usr_input == "remove-on-success")
        {
            string command = receive_input("Input command: ");
            t->delete_on_success(command);
        }
        else if (usr_input == "add-on-failure")
        {
            string command = receive_input("Input command: ");
            t->add_on_failure(command);
        }
        else if (usr_input == "remove-on-failure")
        {
            string command = receive_input("Input command: ");
            t->delete_on_failure(command);
        }
        else if (usr_input == "start")
        {
            t->start();
        }
        else if (usr_input == "stop")
        {
            t->stop();
        }
        else if (usr_input == "help")
        {
            vector<string> commands = {
                "add-command",
                "remove-command",
                "add-path",
                "remove-path",
                "add-on-success",
                "remove-on-success",
                "add-on-failure",
                "remove-on-failure",
                "start",
                "stop",
                "help",
                "exit"};
            print_list("--- COMMANDS ---\n", commands);
        }
        else if (usr_input == "exit")
            break;
        else
        {
            cerr << "INVALID INPUT" << endl;
        }
    }

    cout << "Exiting..." << endl;
    cout << "---------------------------------------------" << endl;
    exit(EXIT_SUCCESS);
    return 0;
}