#include <filesystem>
#include <fstream>
#include <vector>
#include <string>


#include "../../src/include/session_logger.h"
#include "../../src/include/types.h"
#include "../utest.h"
#include "../../src/include/json.hpp"

namespace fs = std::filesystem;
using namespace flowhook;
using namespace std;

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
    WatchEvent e(IN_MODIFY, "file", "/tmp/sl_test_file.txt", 0);
    utest_fixture->er = new ExecutionResult{1, 0, e, "log", vector<string>{"ls"}};
    utest_fixture->er2 = new ExecutionResult{2, 0, e, "log", vector<string>{"ls"}};
    utest_fixture->sl = new SessionLogger();

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
    EXPECT_TRUE(utest_fixture->sl->start("/tmp/sl_test").isOk());
}
UTEST_F(SessionLoggerFixture, start_twice)
{
    EXPECT_TRUE(utest_fixture->sl->start("/tmp/sl_test").isOk());
    EXPECT_TRUE(utest_fixture->sl->start("/tmp/sl_test").isErr());
}
UTEST_F(SessionLoggerFixture, start_nonexistent)
{
    EXPECT_TRUE(utest_fixture->sl->start("/tmp/sl_nonexistent").isErr());
}
UTEST_F(SessionLoggerFixture, start_empty)
{
    EXPECT_TRUE(utest_fixture->sl->start("").isErr());
}

// ---------------------------------------------------------------------------
// LogExecution
// ---------------------------------------------------------------------------
UTEST_F(SessionLoggerFixture, log_execution)
{
    ASSERT_TRUE(utest_fixture->sl->start("/tmp/sl_test").isOk());
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
    ASSERT_TRUE(utest_fixture->sl->start("/tmp/sl_test").isOk());
    EXPECT_TRUE(utest_fixture->sl->stop().isOk());
}
UTEST_F(SessionLoggerFixture, stop_without_start)
{
    EXPECT_TRUE(utest_fixture->sl->stop().isErr());
}
UTEST_F(SessionLoggerFixture, stop_twice)
{
    ASSERT_TRUE(utest_fixture->sl->start("/tmp/sl_test").isOk());
    EXPECT_TRUE(utest_fixture->sl->stop().isOk());
    EXPECT_TRUE(utest_fixture->sl->stop().isErr());
}

UTEST_MAIN()