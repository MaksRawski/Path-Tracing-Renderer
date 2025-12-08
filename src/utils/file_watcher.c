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

// NOTE: exits if the file at path cannot be watched
FileWatcher FileWatcher_new(const char *path) {
  FileWatcher self = {0};

  struct stat stats = {0};
  stat(path, &stats);
  self._mtime = stats.st_mtime;
  self._ctime = stats.st_ctime;

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
  bool did_change =
      (self->_mtime != stats.st_mtime || self->_ctime != stats.st_ctime);
  self->_mtime = stats.st_mtime;
  self->_ctime = stats.st_ctime;

  return did_change;
}

void FileWatcher_delete(FileWatcher *self) {
  free(self->path);
  self = NULL;
}
