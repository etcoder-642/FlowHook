#include <filesystem>
#include <sys/inotify.h>
#include <vector>
#include <string>


#include "../../src/include/session_logger.h"
#include "../../src/include/types.h"
#include "../utest.h"

namespace fs = std::filesystem;
using namespace flowhook;
using namespace std;

bool FLOWHOOK_DEBUG = false;
bool FLOWHOOK_VERBOSE = false;
bool FLOWHOOK_QUIET = false;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
struct SessionLoggerFixture
{
    SessionLogger *sl;
    ExecutionResult *er;
    ExecutionResult *er2;
};

UTEST_F_SETUP(SessionLoggerFixture)
{
    WatchEvent e(0, "file", "/tmp/sl_test_file.txt", IN_MODIFY);
    utest_fixture->er = new ExecutionResult(1, 0, e, "log", vector<string>{"ls"});
    utest_fixture->er2 = new ExecutionResult(2, 0, e, "log", vector<string>{"ls"});
    auto s = SessionLogger::create("/tmp/sl_test_file.txt");
    if(s.isErr())
    {
        cout << s.getErrMessage() << endl;
        exit(1);
    }

    utest_fixture->sl = s.unwrap();

    fs::create_directories("/tmp/sl_test");
}

UTEST_F_TEARDOWN(SessionLoggerFixture)
{
    delete utest_fixture->sl;
    delete utest_fixture->er2;
    delete utest_fixture->er;
    fs::remove_all("/tmp/sl_test");
}

// ---------------------------------------------------------------------------
// Start
// ---------------------------------------------------------------------------
UTEST_F(SessionLoggerFixture, start)
{
    EXPECT_TRUE(utest_fixture->sl->start().isOk());
}
UTEST_F(SessionLoggerFixture, start_twice)
{
    EXPECT_TRUE(utest_fixture->sl->start().isOk());
    EXPECT_TRUE(utest_fixture->sl->start().isOk()); // idempotent
}

// ---------------------------------------------------------------------------
// LogExecution
// ---------------------------------------------------------------------------
UTEST_F(SessionLoggerFixture, log_execution)
{
    ASSERT_TRUE(utest_fixture->sl->start().isOk());
    auto r = utest_fixture->sl->log_execution(*utest_fixture->er);
    EXPECT_TRUE(r.isOk());
}
UTEST_F(SessionLoggerFixture, log_execution_no_running)
{
    auto r = utest_fixture->sl->log_execution(*utest_fixture->er);
    EXPECT_TRUE(r.isErr());
}

// ---------------------------------------------------------------------------
// Stop
// ---------------------------------------------------------------------------
UTEST_F(SessionLoggerFixture, stop)
{
    ASSERT_TRUE(utest_fixture->sl->start().isOk());
    EXPECT_TRUE(utest_fixture->sl->stop().isOk());
}

UTEST_F(SessionLoggerFixture, stop_without_start)
{
    EXPECT_TRUE(utest_fixture->sl->stop().isOk()); // idempotent
}

UTEST_F(SessionLoggerFixture, stop_twice)
{
    ASSERT_TRUE(utest_fixture->sl->start().isOk());
    EXPECT_TRUE(utest_fixture->sl->stop().isOk());
    EXPECT_TRUE(utest_fixture->sl->stop().isOk()); // idempotent
}

UTEST_MAIN()
