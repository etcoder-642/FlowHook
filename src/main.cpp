#include <vector>
#include <string>
#include <iostream>

#include "include/flowhook_core.h"
#include "types.h"

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
   3. add config_manager layer | DONE
   4. add some test cases using utest.h | For tommorrow
   5. split CMakeLists.txt | for tommorrow
      - separate library target from CLI target
      - wire tests target
   6. start working on the CLI layer
   7. add IN_MOVED_TO and IN_MOVED_FROM events
*/

int main()
{
    FlowHookCore core;

    // 1. create task
    std::string name = "build-test";
    std::string dir = "/home/mnasie/coding/C++/fitTrack";
    auto r1 = core.create_task(name, dir);
    if (r1.isErr()) { std::cerr << r1.unwrapErr().message << "\n"; return 1; }

    // 2. add commands via update_task
    Task t;
    t.name = "build-test";
    t.working_directory = dir;
    t.commands = {"g++ main.cpp -o main"};
    t.on_success = {"echo '[OK] build passed'"};
    t.on_failure = {"echo '[FAIL] build failed'"};
    auto r2 = core.update_task(t);
    if (r2.isErr()) { std::cerr << r2.unwrapErr().message << "\n"; return 1; }

    // 3. flush + start
    auto r3 = core.start_task(name);
    if (r3.isErr()) { std::cerr << r3.unwrapErr().message << "\n"; return 1; }

    std::cout << "Watching. Touch a file to trigger.\n";
    std::cin.get(); // block until Enter

    auto r4 = core.stop_task(name);
    if (r4.isErr()) { std::cerr << r4.unwrapErr().message << "\n"; return 1; }

    return 0;
}