#include <filesystem>
#include <fstream>
#include <iostream>

#include "../../src/include/task_runner.h"
#include "../../src/include/types.h"
#include "../utest.h"

namespace fs = std::filesystem;
using namespace flowhook;
using namespace std;

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------

namespace
{
    Result<void> temp_cb(const WatchEvent &e) { return Result<void>::Ok(); }
    Result<void> temp_cb2(const WatchEvent &e) { return Result<void>::Ok(); }
    Result<void> temp_cb3(const WatchEvent &e) { return Result<void>::Ok(); }

}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
struct TaskRunnerFixture
{
    TaskRunner *tr;
    WatchCallback cb;
    WatchCallback cb2;
    WatchCallback cb3;
};

UTEST_F_SETUP(TaskRunnerFixture)
{
    utest_fixture->cb = WatchCallback::from_raw(temp_cb);
    utest_fixture->cb2 = WatchCallback::from_raw(temp_cb2);
    utest_fixture->cb3 = WatchCallback::from_raw(temp_cb3);
    fs::create_directories("/tmp/tr_test");
    fs::create_directories("/tmp/tr_test2");
    auto r = TaskRunner::create("tr_test", "/tmp/tr_test");
    ASSERT_TRUE(r.isOk());
    utest_fixture->tr = r.unwrap();
}

UTEST_F_TEARDOWN(TaskRunnerFixture)
{
    utest_fixture->tr->stop();
    fs::remove_all("/tmp/tr_test");
    fs::remove_all("/tmp/tr_test2");
    fs::remove("/tmp/tr_test_file.txt");
}

// ---------------------------------------------------------------------------
// Create
// ---------------------------------------------------------------------------
UTEST_F(TaskRunnerFixture, create_success)
{
    ASSERT_NE(utest_fixture->tr, nullptr);
}

// ---------------------------------------------------------------------------
// Changing values
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, change_task_name)
{
    ASSERT_TRUE(utest_fixture->tr->change_task_name("new_task_name").isOk());
}
UTEST_F(TaskRunnerFixture, change_task_name_twice)
{
    ASSERT_TRUE(utest_fixture->tr->change_task_name("new_task_name").isOk());
    EXPECT_TRUE(utest_fixture->tr->change_task_name("new_task_name").isErr());
}
UTEST_F(TaskRunnerFixture, change_task_name_empty)
{
    EXPECT_TRUE(utest_fixture->tr->change_task_name("").isErr());
}
UTEST_F(TaskRunnerFixture, change_working_directory)
{
    ASSERT_TRUE(utest_fixture->tr->change_working_directory("/tmp/tr_test2").isOk());
}
UTEST_F(TaskRunnerFixture, change_working_directory_twice)
{
    ASSERT_TRUE(utest_fixture->tr->change_working_directory("/tmp/tr_test2").isOk());
    EXPECT_TRUE(utest_fixture->tr->change_working_directory("/tmp/tr_test2").isErr());
}
UTEST_F(TaskRunnerFixture, change_working_directory_nonexistent)
{
    EXPECT_TRUE(utest_fixture->tr->change_working_directory("/tmp/tr_test_nonexistent").isErr());
}
UTEST_F(TaskRunnerFixture, change_working_directory_empty)
{
    EXPECT_TRUE(utest_fixture->tr->change_working_directory("").isErr());
}

// ---------------------------------------------------------------------------
// AddCommand
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, add_command)
{
    EXPECT_TRUE(utest_fixture->tr->add_command("ls").isOk());
}
UTEST_F(TaskRunnerFixture, add_command_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->add_command("ls").isErr());
}
UTEST_F(TaskRunnerFixture, add_command_empty)
{
    EXPECT_TRUE(utest_fixture->tr->add_command("").isErr());
}

// ---------------------------------------------------------------------------
// DeleteCommand
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, delete_command)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_command("ls").isOk());
}
UTEST_F(TaskRunnerFixture, delete_command_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->delete_command("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_command("ls").isErr());
}
UTEST_F(TaskRunnerFixture, delete_command_empty)
{
    EXPECT_TRUE(utest_fixture->tr->delete_command("").isErr());
}

// ---------------------------------------------------------------------------
// AddPath
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, add_path)
{
    EXPECT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
}
UTEST_F(TaskRunnerFixture, add_path_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    EXPECT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isErr());
}
UTEST_F(TaskRunnerFixture, add_path_empty)
{
    EXPECT_TRUE(utest_fixture->tr->add_path("").isErr());
}
UTEST_F(TaskRunnerFixture, add_path_nonexistent)
{
    EXPECT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_nonexistent").isErr());
}
UTEST_F(TaskRunnerFixture, add_path_file)
{
    ofstream("/tmp/tr_test_file.txt").close();
    EXPECT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_file.txt").isOk());
}

// ---------------------------------------------------------------------------
// DeletePath
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, delete_path)
{
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_path("/tmp/tr_test").isOk());
}
UTEST_F(TaskRunnerFixture, delete_path_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    ASSERT_TRUE(utest_fixture->tr->delete_path("/tmp/tr_test").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_path("/tmp/tr_test").isErr());
}
UTEST_F(TaskRunnerFixture, delete_path_empty)
{
    EXPECT_TRUE(utest_fixture->tr->delete_path("").isErr());
}

// ---------------------------------------------------------------------------
// AddOnSuccess
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, add_on_success)
{
    EXPECT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
}
UTEST_F(TaskRunnerFixture, add_on_success_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->add_on_success("ls").isErr());
}
UTEST_F(TaskRunnerFixture, add_on_success_empty)
{
    EXPECT_TRUE(utest_fixture->tr->add_on_success("").isErr());
}

// ---------------------------------------------------------------------------
// DeleteOnSuccess
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, delete_on_success)
{
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_on_success("ls").isOk());
}
UTEST_F(TaskRunnerFixture, delete_on_success_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->delete_on_success("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_on_success("ls").isErr());
}
UTEST_F(TaskRunnerFixture, delete_on_success_empty)
{
    EXPECT_TRUE(utest_fixture->tr->delete_on_success("").isErr());
}

// ---------------------------------------------------------------------------
// AddOnFailure
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, add_on_failure)
{
    EXPECT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
}
UTEST_F(TaskRunnerFixture, add_on_failure_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->add_on_failure("ls").isErr());
}
UTEST_F(TaskRunnerFixture, add_on_failure_empty)
{
    EXPECT_TRUE(utest_fixture->tr->add_on_failure("").isErr());
}

// ---------------------------------------------------------------------------
// DeleteOnFailure
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, delete_on_failure)
{
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_on_failure("ls").isOk());
}
UTEST_F(TaskRunnerFixture, delete_on_failure_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->delete_on_failure("ls").isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_on_failure("ls").isErr());
}
UTEST_F(TaskRunnerFixture, delete_on_failure_empty)
{
    EXPECT_TRUE(utest_fixture->tr->delete_on_failure("").isErr());
}

// ---------------------------------------------------------------------------
// AddCallback
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, add_callback_member_function)
{
    WatchCallback cb = {utest_fixture->tr, &TaskRunner::execute};
    EXPECT_TRUE(utest_fixture->tr->add_callback(cb).isOk());
}
UTEST_F(TaskRunnerFixture, add_callback_raw_function)
{
    EXPECT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
}
UTEST_F(TaskRunnerFixture, add_callback_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isErr());
}
UTEST_F(TaskRunnerFixture, add_callback_empty)
{
    EXPECT_TRUE(utest_fixture->tr->add_callback(WatchCallback()).isErr());
}
UTEST_F(TaskRunnerFixture, add_callback_multiple_callbacks)
{
    EXPECT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb3).isOk());
}

// ---------------------------------------------------------------------------
// DeleteCallback
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, delete_callback)
{
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_callback(utest_fixture->cb).isOk());
}
UTEST_F(TaskRunnerFixture, delete_callback_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->tr->delete_callback(utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->tr->delete_callback(utest_fixture->cb).isErr());
}
UTEST_F(TaskRunnerFixture, delete_callback_empty)
{
    EXPECT_TRUE(utest_fixture->tr->delete_callback(WatchCallback()).isErr());
}

// ---------------------------------------------------------------------------
// Execute
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, execute)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    ofstream("/tmp/tr_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_file.txt").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    WatchEvent e(2, "file", "/tmp/tr_test_file.txt", 0);
    auto r = utest_fixture->tr->execute(e);
    EXPECT_TRUE(r.isOk());
}
UTEST_F(TaskRunnerFixture, execute_empty)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    ofstream("/tmp/tr_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_file.txt").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->tr->execute(WatchEvent()).isErr());
}

// ---------------------------------------------------------------------------
// Start
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, start)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    ofstream("/tmp/tr_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_file.txt").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->tr->start().isOk());
}
UTEST_F(TaskRunnerFixture, start_twice)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    ofstream("/tmp/tr_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_file.txt").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->tr->start().isOk());
    EXPECT_TRUE(utest_fixture->tr->start().isErr());
}

// ---------------------------------------------------------------------------
// Stop
// ---------------------------------------------------------------------------

UTEST_F(TaskRunnerFixture, stop)
{
    ASSERT_TRUE(utest_fixture->tr->add_command("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test").isOk());
    ofstream("/tmp/tr_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->tr->add_path("/tmp/tr_test_file.txt").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_success("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_on_failure("ls").isOk());
    ASSERT_TRUE(utest_fixture->tr->add_callback(utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->tr->start().isOk());
    auto r = utest_fixture->tr->stop();
    EXPECT_TRUE(r.isOk());
    if(r.isErr())
    {
        cerr << r.unwrapErr().message << endl;
    }
}
UTEST_F(TaskRunnerFixture, stop_without_start)
{
    EXPECT_TRUE(utest_fixture->tr->stop().isErr());
}

UTEST_MAIN()