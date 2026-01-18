#include "utils/file_watcher.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if _POSIX_C_SOURCE >= 200809L
#define st_mtimensec st_mtim.tv_nsec
#endif

#include <sys/stat.h>
#ifdef _WIN32
#define stat _stat
#endif

// NOTE: using nanosecond precision on linux and second precision elsewhere
// (which could very well be problematic but I don't care about that for now)
// TODO: instead use OS specific APIs that offer more reliable mechanisms for
// this; inotify on Linux and ReadDirectoryChangesW on Windows

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

  struct stat stats;
  if (stat(path, &stats) == -1) {
    perror("stat");
  };
  set_stats(&self, stats);

  strcpy(self.path, path);

  return self;
}

// NOTE: exits if the file becomes unreadable
bool FileWatcher_did_change(FileWatcher *self) {
  struct stat stats;
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

void FileWatcher_delete(FileWatcher *self) { (void)(self); }
