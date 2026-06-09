#include <iostream>
#include <string>

#include "../include/display.h"
#include "../include/filewatcher.h"

using namespace std;

namespace l_fw
{
    void print_header()
    {
        cout << "===================================================" << endl;
        cout << "------------  FLOWHOOK v0.0.1  -------------------" << endl;
        cout << "===================================================" << endl;
    }

    string receive_input(const string &msg)
    {
        cout << msg;
        string text_buffer;
        getline(cin, text_buffer);
        return text_buffer;
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
