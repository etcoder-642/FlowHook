#include <vector>
#include <string>
#include <iostream>

#include "include/flowhook_core.h"
#include "include/types.h"
#include "include/macros.hpp"

using namespace std;
using namespace flowhook;

/*
   THINGS TO WORK ON tomorrow:
   1. finish adding load() for config_manager()
   2. finish a load integration for flowhook_core and ini
   4. add some test cases using utest.h | For tommorrow
   5. split CMakeLists.txt | for tommorrow
      - separate library target from CLI target
      - wire tests target
   6. start working on the CLI layer
   7. add IN_MOVED_TO and IN_MOVED_FROM events
*/

int main()
{
    auto result = FlowHookCore::create();
    if(result.isErr()){
        cerr << "[FAIL] FlowHookCore::create() failed: " << result.unwrapErr().message << "\n";
        return 1;
    }
    auto core = result.unwrap();
    core->init();

    auto check = [](auto r, const string &label) -> bool {
        if (r.isErr()) {
            cerr << "[FAIL] " << label << ": " << r.unwrapErr().message << "\n";
            r.unwrapErr().printStackTrace();
            return false;
        }
        cout << "[OK] " << label << "\n";
        return true;
    };

    // 1. create tasks
    string name1 = "build-test";
    string name2 = "lint-test";
    string dir = "/home/mnasie/coding/C++/fitTrack/";

    if (!check(core->create_task(name1, dir), "create task: " + name1)) return 1;
    if (!check(core->create_task(name2, dir), "create task: " + name2)) return 1;

    // 2. duplicate creation should fail
    auto dup = core->create_task(name1, dir);
    if (!dup.isErr()) { cerr << "[FAIL] duplicate task was not rejected\n"; return 1; }
    cout << "[OK] duplicate rejected: " << dup.unwrapErr().message << "\n";

    if (!check(core->set_task_path(name1, dir), "set path: " + name1)) return 1;

    // 3. update tasks with commands
    string _command = "g++ main.cpp -o main";
    string _on_success = "ffplay -nodisp -autoexit -loglevel quiet success.mp3";
    string _on_failure = "ffplay -nodisp -autoexit -loglevel quiet fahh_meme.mp3";
    if (!check(core->set_task_command(name1, _command), "set task command: " + name1)) return 1;
    if (!check(core->set_task_on_success(name1, _on_success), "set task on_success: " + name1)) return 1;
    if (!check(core->set_task_on_failure(name1, _on_failure), "set task on_failure" + name1)) return 1;

    // 5. start tasks
    if (!check(core->start_task(name1), "start task: " + name1)) return 1;

    // 6. start already running task should fail or be idempotent
    auto double_start = core->start_task(name1);
    if (double_start.isErr())
        cout << "[OK] double start rejected: " << double_start.unwrapErr().message << "\n";
    else
        cout << "[NOTE] double start was silently accepted\n";

    cout << "\nWatching task. Touch a file in " << dir << " to trigger.\nPress Enter to stop.\n";
    cin.get();

    // 7. stop tasks
    if (!check(core->stop_task(name1), "stop task: " + name1)) return 1;
    if (!check(core->stop_task(name2), "stop task: " + name2)) return 1;

    // 8. stop nonexistent task
    string ghost_name = "ghost-task";
    auto bad_stop = core->stop_task(ghost_name);
    if (!bad_stop.isErr()) { cerr << "[FAIL] ghost stop was not rejected\n"; return 1; }
    cout << "[OK] ghost stop rejected: " << bad_stop.unwrapErr().message << "\n";

    // 9. delete one task
    if (!check(core->delete_task(name2), "delete task: " + name2)) return 1;
    cout << "\nAll tests passed.\n";
    return 0;
}