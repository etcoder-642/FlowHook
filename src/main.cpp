#include <vector>
#include <string>
#include <iostream>

#include <stdio.h>
#include <fstream>

#include "../include/filewatcher.h"
#include "../include/display.h"
#include "../include/task_watcher.h"
#include "../include/session_logger.h"
#include "../include/error/error.h"
#include "../include/error/result.h"

using namespace std;
using namespace l_fw;


TaskWatcher tw("test", "/home/mnasie/coding/C++/fitTrack");

void execute_cmd(const _i_event &e)
{
    tw.execute(e);
}

int main(int argc, char *argv[])
{

    string usr_input;
    print_header();
    string prompt = "g++ main.cpp -o main";
    tw.add_command(prompt);
    string success_cmd = "ffplay -nodisp -autoexit -loglevel quiet success.mp3";
    tw.add_on_success(success_cmd);
    string failure_cmd = "ffplay -nodisp -autoexit -loglevel quiet fahh_meme.mp3";
    tw.add_on_failure(failure_cmd);
    string path = "/home/mnasie/coding/C++/fitTrack/main.cpp";
    tw.add_path(path);

    while (usr_input != "exit")
    {
        usr_input = receive_input("Input Value(help -- to see all commands): ");
        if (usr_input == "add-command")
        {
            string path = receive_input("Input Command: ");
            tw.add_command(path);
        }
        else if (usr_input == "remove-command")
        {
            string path = receive_input("Input Command: ");
            tw.delete_command(path);
        }
        else if (usr_input == "add-path")
        {
            string path = receive_input("Input file/directory path: ");
            tw.add_path(path);
        }
        else if (usr_input == "remove-path")
        {
            string path = receive_input("Input file/directory path: ");
            tw.delete_path(path);
        }
        else if (usr_input == "add-on-success")
        {
            string command = receive_input("Input command: ");
            tw.add_on_success(command);
        }
        else if (usr_input == "remove-on-success")
        {
            string command = receive_input("Input command: ");
            tw.delete_on_success(command);
        }
        else if (usr_input == "add-on-failure")
        {
            string command = receive_input("Input command: ");
            tw.add_on_failure(command);
        }
        else if (usr_input == "remove-on-failure")
        {
            string command = receive_input("Input command: ");
            tw.delete_on_failure(command);
        }
        else if (usr_input == "start")
        {
            tw.start(execute_cmd);
        }
        else if (usr_input == "stop")
        {
            tw.stop(execute_cmd);
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
                "exit"
            };
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