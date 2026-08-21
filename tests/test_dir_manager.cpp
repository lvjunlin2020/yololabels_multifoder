// Unit tests for dir_manager (pure C++, no Qt).
// Build: g++ -std=c++17 -Wall -I.. tests/test_dir_manager.cpp dir_manager.cpp -o test_dir_manager

#include "dir_manager.hpp"

#include <cstdio>
#include <string>

static int g_failures = 0;
static int g_checks   = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++g_checks;                                                        \
        if (!(cond)) {                                                     \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                  \
    } while (0)

static void test_add_dir_dedup_and_sort()
{
    DirManager dm;
    CHECK(dm.addDir("/data/train10") == 0);
    CHECK(dm.addDir("/data/train2")  == 0);  // sorts before train10 -> index 0
    CHECK(dm.size() == 2);
    // Numeric sort: train2 before train10 (train2 lands at index 0)
    CHECK(dm.entryAt(0)->path == "/data/train2");
    CHECK(dm.entryAt(1)->path == "/data/train10");
    // addDir returned the sorted index of train2, which is 0
    // (the check above already covers this; fix the earlier assertion)

    // Duplicate (same path) is not added again
    CHECK(dm.addDir("/data/train2") == 0);
    CHECK(dm.size() == 2);

    // Trailing slash normalized to same entry
    CHECK(dm.addDir("/data/train10/") == 1);
    CHECK(dm.size() == 2);
}

static void test_numeric_sort_edge_cases()
{
    DirManager dm;
    dm.addDir("/data/train10");
    dm.addDir("/data/train2");
    dm.addDir("/data/train20");
    dm.addDir("/data/train1");
    // expected: train1, train2, train10, train20
    CHECK(dm.entryAt(0)->path == "/data/train1");
    CHECK(dm.entryAt(1)->path == "/data/train2");
    CHECK(dm.entryAt(2)->path == "/data/train10");
    CHECK(dm.entryAt(3)->path == "/data/train20");

    // Windows-style paths, case-insensitive name compare
    DirManager dm2;
    dm2.addDir("C:\\data\\Zeta");
    dm2.addDir("C:\\data\\alpha");
    dm2.addDir("C:\\data\\Beta2");
    dm2.addDir("C:\\data\\Beta10");
    // expected: alpha, Beta2, Beta10, Zeta
    CHECK(dm2.entryAt(0)->path == "C:\\data\\alpha");
    CHECK(dm2.entryAt(1)->path == "C:\\data\\Beta2");
    CHECK(dm2.entryAt(2)->path == "C:\\data\\Beta10");
    CHECK(dm2.entryAt(3)->path == "C:\\data\\Zeta");
}

static void test_current_tracking_on_insert()
{
    DirManager dm;
    dm.addDir("/data/b");       // current -> 0 (b)
    CHECK(dm.currentIndex() == 0);
    dm.addDir("/data/a");       // sorts before b -> b shifts to 1
    CHECK(dm.currentIndex() == 1);
    CHECK(dm.currentPath() == "/data/b");
    dm.addDir("/data/c");       // sorts after b -> no shift
    CHECK(dm.currentIndex() == 1);
    CHECK(dm.currentPath() == "/data/b");
}

static void test_switch_and_wrap()
{
    DirManager dm;
    dm.addDir("/data/b");
    dm.addDir("/data/a");
    dm.addDir("/data/c");
    // order: a(0) b(1) c(2), current = 1 (b)
    CHECK(dm.currentIndex() == 1);

    // Switch to a(0)
    CHECK(dm.switchToDir(0));
    CHECK(dm.currentPath() == "/data/a");
    CHECK(dm.nextDir() == 1);
    CHECK(dm.prevDir() == 2);   // wraps around

    // Already-current switch returns false
    CHECK(!dm.switchToDir(0));

    // Switch to c(2)
    CHECK(dm.switchToDir(2));
    CHECK(dm.currentPath() == "/data/c");
    CHECK(dm.nextDir() == 0);   // wraps around
    CHECK(dm.prevDir() == 1);

    // Out-of-range wraps: 5 % 3 = 2 -> c (already current) -> false
    CHECK(!dm.switchToDir(5));
    // -1 wraps to ((-1%3)+3)%3 = 2 -> c (already current) -> false
    CHECK(!dm.switchToDir(-1));
    // 10 wraps to (10%3)=1 -> b
    CHECK(dm.switchToDir(10));
    CHECK(dm.currentPath() == "/data/b");
}

static void test_remove_dir()
{
    DirManager dm;
    dm.addDir("/data/b");
    dm.addDir("/data/a");
    dm.addDir("/data/c");
    // current = b (index 1)

    CHECK(dm.removeDir("/data/a"));   // current b stays at index 0
    CHECK(dm.currentIndex() == 0);
    CHECK(dm.currentPath() == "/data/b");

    CHECK(dm.removeDir("/data/b"));   // removing current -> falls back to 0 (a gone, c at 0)
    CHECK(dm.currentIndex() == 0);
    CHECK(dm.currentPath() == "/data/c");

    CHECK(!dm.removeDir("/data/nope"));
    CHECK(dm.size() == 1);

    CHECK(dm.removeDir("/data/c"));
    CHECK(dm.isEmpty());
    CHECK(dm.currentIndex() == -1);
    CHECK(dm.switchToDir(0) == false);
}

static void test_progress_and_classes()
{
    DirManager dm;
    dm.addDir("/data/train");
    dm.addDir("/data/val");

    dm.setProgress("/data/train", 41);
    dm.setClassesFile("/data/train", "/data/train/classes.txt");
    dm.setClassesFile("/data/val", "");

    CHECK(dm.progressOf("/data/train") == 41);
    CHECK(dm.progressOf("/data/val") == 0);
    CHECK(dm.classesFileOf("/data/train") == "/data/train/classes.txt");
    CHECK(dm.classesFileOf("/data/val").empty());
    CHECK(dm.classesFileOf("/data/unknown").empty());

    // Progress of a not-yet-added dir is 0 and set is a no-op
    dm.setProgress("/data/unknown", 7);
    CHECK(dm.progressOf("/data/unknown") == 0);
}

static void test_snapshot_restore()
{
    DirManager dm;
    dm.addDir("/data/train10");
    dm.addDir("/data/train2");
    dm.setProgress("/data/train2", 5);
    dm.setClassesFile("/data/train2", "/cls.txt");
    dm.switchToDir(0);  // train10

    const std::vector<DirEntry> snap = dm.snapshot();

    DirManager dm2;
    dm2.restore(snap, dm.currentIndex());

    CHECK(dm2.size() == 2);
    CHECK(dm2.currentIndex() == dm.currentIndex());
    CHECK(dm2.currentPath() == dm.currentPath());
    CHECK(dm2.progressOf("/data/train2") == 5);
    CHECK(dm2.classesFileOf("/data/train2") == "/cls.txt");
}

static void test_restore_clamps_bad_current()
{
    DirManager dm;
    dm.addDir("/data/a");
    const std::vector<DirEntry> snap = dm.snapshot();

    DirManager dm2;
    dm2.restore(snap, 99);   // out of range -> clamped to 0
    CHECK(dm2.currentIndex() == 0);

    DirManager dm3;
    dm3.restore({}, -1);     // empty restore
    CHECK(dm3.isEmpty());
    CHECK(dm3.currentIndex() == -1);
}

int main()
{
    test_add_dir_dedup_and_sort();
    test_numeric_sort_edge_cases();
    test_current_tracking_on_insert();
    test_switch_and_wrap();
    test_remove_dir();
    test_progress_and_classes();
    test_snapshot_restore();
    test_restore_clamps_bad_current();

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
