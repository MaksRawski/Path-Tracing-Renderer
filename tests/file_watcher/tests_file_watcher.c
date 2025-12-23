#include "file_watcher/tests_file_watcher.h"
#include "asserts.h"
#include "tests_macros.h"
#include "utils/file_watcher.h"
#include <stdio.h>
#include <unistd.h>

bool test_FileWatcher__basic(void) {
  const char *path = "test_FileWatcher__basic.txt";
  FILE *f = fopen(path, "w");
  FileWatcher fw = FileWatcher_new(path);
  ASSERT_CUSTOM(f != NULL, "Failed to open file!");

  ASSERT_CUSTOM(!FileWatcher_did_change(&fw), "No changes were yet expected.");
  ASSERT_CUSTOM(!FileWatcher_did_change(&fw), "No changes were yet expected.");
  fputs("A", f);
  fflush(f);
  ASSERT_CUSTOM(FileWatcher_did_change(&fw), "Expected a change!");

  fclose(f);
  ASSERT_CUSTOM_FMT(remove(path) == 0,
                    "[cleanup] Failed to delete the test file: %s", path);
  return true;
}

bool all_filewatcher_tests(void) {
  bool ok = true;
  TEST_RUN(test_FileWatcher__basic, &ok);
  return ok;
}
