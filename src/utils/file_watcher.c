#include "utils/file_watcher.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#include <sys/stat.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define stat _stat
#endif

// NOTE: using nanosecond precision on linux and second precision elsewhere
// (which could very well be problematic but I don't care about that for now)
// TODO: instead use OS specific APIs that offer more reliable mechanisms for
// this

void set_stats(FileWatcher *self, struct stat stats);
bool stats_changed(FileWatcher *self, struct stat stats);

void set_stats(FileWatcher *self, struct stat stats) {
#ifdef __linux__
  self->_mtimensec = stats.st_mtimensec;
#else
  self->_mtime = stats.st_mtime;
#endif
}

bool stats_changed(FileWatcher *self, struct stat stats) {
#ifdef __linux__
  return self->_mtimensec != stats.st_mtimensec;
#else
  return self->_mtime != stats.st_mtime;
#endif
}

// NOTE: exits if the file at path cannot be watched
FileWatcher FileWatcher_new(const char *path) {
  FileWatcher self = {0};

  struct stat stats = {0};
  stat(path, &stats);
  set_stats(&self, stats);

  self.path = (char *)malloc(strlen(path) + 1);
  strcpy(self.path, path);

  return self;
}

// NOTE: exits if the file becomes unreadable
bool FileWatcher_did_change(FileWatcher *self) {
  struct stat stats = {0};
  if (stat(self->path, &stats) == -1) {
    fprintf(stderr, "Failed to stat %s: %s\n", self->path, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (stats_changed(self, stats)) {
    set_stats(self, stats);
    return true;
  }
  return false;
}

void FileWatcher_delete(FileWatcher *self) {
  free(self->path);
  self = NULL;
}
