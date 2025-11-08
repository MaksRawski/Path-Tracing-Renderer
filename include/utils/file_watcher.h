#ifndef FILE_WATCHER_H_
#define FILE_WATCHER_H_
#include <stdbool.h>
#include <time.h>

typedef struct {
  char *path;
  time_t _mtime;
  time_t _ctime;
} FileWatcher;

FileWatcher FileWatcher_new(const char * path);
bool FileWatcher_did_change(FileWatcher *self);
void FileWatcher_delete(FileWatcher *self);

#endif // FILE_WATCHER_H_
