#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <stdexcept>

#include "filewatcher.h"

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
    fw.on_event(IN_MODIFY, print_modified);
    fw.start(100);

    cout << "---------------------------------------------" << endl;
    cout << "LINUX FILE WATCHER v1.0.0" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Input Value(add, remove, exit): ";
    string input;
    while (true)
    {
        cin >> input;
        cout << "You entered: " << input << endl;
        if (input == "add")
        {
            cout << "Input path: ";
            string path;
            cin >> path;
            fw.add_path(path);
        } else if (input == "remove")
        {
            cout << "Input path: ";
            string path;
            cin >> path;
            fw.remove_path(path);
        } else if (input == "exit")
        {
            break;
        }
    }
    cout << "Exiting..." << endl;
    fw.stop();
    cout << "---------------------------------------------" << endl;
    exit(EXIT_SUCCESS);
    return 0;
}