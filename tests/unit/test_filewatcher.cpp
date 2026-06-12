#include <filesystem>
#include <fstream>
#include <iostream>

#include "../../src/include/filewatcher.h"
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

struct FileWatcherFixture
{
    FileWatcher *fw;
    WatchCallback cb;
    WatchCallback cb2;
    WatchCallback cb3;
};

UTEST_F_SETUP(FileWatcherFixture)
{
    utest_fixture->cb = WatchCallback::from_raw(temp_cb);
    utest_fixture->cb2 = WatchCallback::from_raw(temp_cb2);
    utest_fixture->cb3 = WatchCallback::from_raw(temp_cb3);
    fs::create_directories("/tmp/fh_test");
    fs::create_directories("/tmp/fh_test1");
    fs::create_directories("/tmp/fh_test2");
    fs::create_directories("/tmp/fh_test3");

    auto r = FileWatcher::create();
    ASSERT_TRUE(r.isOk());
    utest_fixture->fw = r.unwrap();
}

UTEST_F_TEARDOWN(FileWatcherFixture)
{
    utest_fixture->fw->stop();
    fs::remove_all("/tmp/fh_test");
    fs::remove_all("/tmp/fh_test1");
    fs::remove_all("/tmp/fh_test2");
    fs::remove_all("/tmp/fh_test3");
    fs::remove("/tmp/fh_test_file.txt");
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, create_success)
{
    ASSERT_NE(utest_fixture->fw, nullptr);
}

// ---------------------------------------------------------------------------
// AddPath
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, add_path_normal)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
}
UTEST_F(FileWatcherFixture, add_multiple_paths)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test1").isOk());
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test2").isOk());
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test3").isOk());
}
UTEST_F(FileWatcherFixture, add_nonexistent_path)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_nonexistent").isErr());
}
UTEST_F(FileWatcherFixture, add_path_twice)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isErr());
}
UTEST_F(FileWatcherFixture, add_empty_path)
{
    EXPECT_TRUE(utest_fixture->fw->add_path("").isErr());
}
UTEST_F(FileWatcherFixture, add_path_file)
{
    ofstream("/tmp/fh_test_file.txt").close();
    EXPECT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test_file.txt").isOk());
}

// ---------------------------------------------------------------------------
// RemovePath
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, remove_path)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test").isOk());
}
UTEST_F(FileWatcherFixture, remove_nonexistent_path)
{
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_nonexistent").isErr());
}
UTEST_F(FileWatcherFixture, remove_path_twice)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test").isErr());
}
UTEST_F(FileWatcherFixture, remove_empty_path)
{
    EXPECT_TRUE(utest_fixture->fw->remove_path("").isErr());
}
UTEST_F(FileWatcherFixture, remove_multiple_paths)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test1").isOk());
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test2").isOk());
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test3").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test1").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test2").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test3").isOk());
}
UTEST_F(FileWatcherFixture, remove_path_file)
{
    ofstream("/tmp/fh_test_file.txt").close();
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test_file.txt").isOk());
    EXPECT_TRUE(utest_fixture->fw->remove_path("/tmp/fh_test_file.txt").isOk());
}

// ---------------------------------------------------------------------------
// LinkEvent
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, link_event)
{
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
}
UTEST_F(FileWatcherFixture, link_event_twice)
{
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isErr());
}
UTEST_F(FileWatcherFixture, link_invalid_event)
{
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_CREATE, utest_fixture->cb).isErr());
}
UTEST_F(FileWatcherFixture, link_different_events)
{
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_CLOSE_WRITE, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MOVED_TO, utest_fixture->cb).isOk());
}
UTEST_F(FileWatcherFixture, link_multiple_callbacks_same_event)
{
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb3).isOk());
}
UTEST_F(FileWatcherFixture, link_multiple_callbacks_multiple_events)
{
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_CLOSE_WRITE, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MOVED_TO, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb3).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_CLOSE_WRITE, utest_fixture->cb3).isOk());
    EXPECT_TRUE(utest_fixture->fw->link_event(IN_MOVED_TO, utest_fixture->cb3).isOk());
}

// ---------------------------------------------------------------------------
// UnlinkEvent
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, unlink_event)
{
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MODIFY, utest_fixture->cb).isOk());
}
UTEST_F(FileWatcherFixture, unlink_event_twice)
{
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->fw->unlink_event(IN_MODIFY, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MODIFY, utest_fixture->cb).isErr());
}
UTEST_F(FileWatcherFixture, unlink_invalid_event)
{
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_CREATE, utest_fixture->cb).isErr());
}
UTEST_F(FileWatcherFixture, unlink_never_linked_callback)
{
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_CLOSE_WRITE, utest_fixture->cb).isErr());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MOVED_TO, utest_fixture->cb).isErr());
}
UTEST_F(FileWatcherFixture, unlink_multiple_callbacks)
{
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb2).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_CLOSE_WRITE, utest_fixture->cb2).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MOVED_TO, utest_fixture->cb2).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb3).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_CLOSE_WRITE, utest_fixture->cb3).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MOVED_TO, utest_fixture->cb3).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MODIFY, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_CLOSE_WRITE, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MOVED_TO, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MODIFY, utest_fixture->cb3).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_CLOSE_WRITE, utest_fixture->cb3).isOk());
    EXPECT_TRUE(utest_fixture->fw->unlink_event(IN_MOVED_TO, utest_fixture->cb3).isOk());
}

// ---------------------------------------------------------------------------
// GetWatchList
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, get_watch_list_empty)
{
    EXPECT_TRUE(utest_fixture->fw->get_watch_list().isOk());
}
UTEST_F(FileWatcherFixture, get_watch_list_after_add)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    EXPECT_TRUE(utest_fixture->fw->get_watch_list().isOk());
}

// ---------------------------------------------------------------------------
// Start / Stop
// ---------------------------------------------------------------------------

UTEST_F(FileWatcherFixture, start_normal)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->start(10).isOk());
}
UTEST_F(FileWatcherFixture, start_twice)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->fw->start(10).isOk());
    EXPECT_TRUE(utest_fixture->fw->start(10).isErr());
}
UTEST_F(FileWatcherFixture, stop_normal)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->fw->start(10).isOk());
    EXPECT_TRUE(utest_fixture->fw->stop().isOk());
}
UTEST_F(FileWatcherFixture, stop_without_start)
{
    EXPECT_TRUE(utest_fixture->fw->stop().isErr());
}
UTEST_F(FileWatcherFixture, start_with_multiple_paths)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test1").isOk());
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test2").isOk());
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test3").isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    EXPECT_TRUE(utest_fixture->fw->start(10).isOk());
}
UTEST_F(FileWatcherFixture, start_with_multiple_callbacks)
{
    ASSERT_TRUE(utest_fixture->fw->add_path("/tmp/fh_test").isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MODIFY, utest_fixture->cb2).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_CLOSE_WRITE, utest_fixture->cb2).isOk());
    ASSERT_TRUE(utest_fixture->fw->link_event(IN_MOVED_TO, utest_fixture->cb2).isOk());
    EXPECT_TRUE(utest_fixture->fw->start(10).isOk());
}

UTEST_MAIN()