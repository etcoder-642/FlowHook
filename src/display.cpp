#include <iostream>
#include <string>

#include "../include/display.h"
#include "../include/filewatcher.h"

using namespace std;

namespace l_fw
{
    void print_header()
    {
        cout << "---------------------------------------------" << endl;
        cout << "FLOWHOOK v1.0.0" << endl;
        cout << "---------------------------------------------" << endl;
    }

    string receive_input(const string &msg)
    {
        cout << "---------------------------------------------" << endl;
        cout << msg;
        string usr_choice;
        getline(cin, usr_choice);
        return usr_choice;
    }

    void print_list(const string &msg, const vector<string> &list)
    {
        cout << "__________________________________________________" << endl;
        cout << msg << endl;
        for (auto &path : list)
        {
            cout << path << endl;
        }
    }

    void print_numbered_list(const string msg, const vector<string> &list)
    {
        cout << "__________________________________________________" << endl;
        cout << msg << endl;
        int i = 1;
        for (auto &path : list)
        {
            cout << "[" << i << "] " << path << endl;
            i++;
        }
    }
}
