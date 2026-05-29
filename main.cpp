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

void print_modified(_i_event e){
    if(e.event_mask == IN_MODIFY){
        cout << "File modified: " << e.path << " [ " << e.filetype << "]" << endl;
    }
}

int main(int argc, char *argv[])
{
    FileWatcher fw;
    for(int i = 0; i < argc; i++){
        string path(argv[i]);
        fw.add_path(path);
    }
    fw.on_event(IN_MODIFY, print_modified);
    fw.start(-1);

    exit(EXIT_SUCCESS);
    
    return 0;
}