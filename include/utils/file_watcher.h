#ifndef FILE_WATCHER_H_
#define FILE_WATCHER_H_
#include <stdbool.h>

typedef struct {
  char path[1024];
#ifdef __linux__
  unsigned long _mtimensec;
#else
  long _mtime;
#endif
} FileWatcher;

FileWatcher FileWatcher_new(const char *path);
bool FileWatcher_did_change(FileWatcher *self);
void FileWatcher_delete(FileWatcher *self);

#endif // FILE_WATCHER_H_
