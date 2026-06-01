#include <vector>
#include <string>
#include <iostream>
#include <sys/inotify.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <fstream>

#include "../include/filewatcher.h"
#include "../include/display.h"

using namespace std;
using namespace l_fw;

void print_modified(_i_event e)
{

    cout << "[printing modify event...]" << endl;
    if (e.event_mask == IN_MODIFY)
    {
        cout << "\nFile modified: " << e.path << " [ " << e.filetype << "]" << endl;
    }
}

int main(int argc, char *argv[])
{
    FileWatcher fw;
    for (int i = 1; i < argc; i++)
    {
        string path(argv[i]);
        fw.add_path(path);
    }
    fw.link_event(IN_MODIFY, print_modified);
    fw.start(100);

    string usr_input;
    while (usr_input != "exit")
    {
        print_header();
        usr_input = receive_input("Input Value(add, remove, exit): ");

        if (usr_input == "add")
        {
            string path = receive_input("Input file/directory path: ");
            fw.add_path(path);
        }
        else if (usr_input == "remove")
        {
            string path = receive_input("Input file/directory path: ");
            fw.remove_path(path);
        } 
        else if(usr_input == "list")
        {
            auto list = fw.get_watch_list();
            if(list.isErr())
            {
                printf(list.getErrMessage().c_str());
                return 0;
            }

            print_list("ALL WATCHED FILES", list.unwrap());
        }
        else
        {
            if(usr_input == "exit") continue;
            cerr << "INVALID INPUT" << endl;
        }
    }

    cout << "Exiting..." << endl;
    fw.stop();
    cout << "---------------------------------------------" << endl;
    exit(EXIT_SUCCESS);
    return 0;
}