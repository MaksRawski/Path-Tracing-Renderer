#ifndef FILE_DIALOG_H_
#define FILE_DIALOG_H_

/// Utilities for opening native file dialog on Windows, macOS and Linux.

#include <stdbool.h>
#include <stddef.h>

bool FileDialog_is_available(void);

// NOTE: these obviously should correspond to the implementations of FileDialog_{open,new}
#ifdef __linux__
#define FileDialog_not_available_msg "'zenity' not installed"
#elif defined(__APPLE__)
#define FileDialog_not_available_msg "'osascript' not available"
#elif defined(_WIN32)
#define FileDialog_not_available_msg "UNREACHABLE"
#endif

/// Returns true on success, logs and returns false on error.
/// Opens native file dialog and asks the user to choose an existing file.
/// NOTE: assumes glTF file filter
bool FileDialog_open_gltf(char *out_path, size_t capacity);

/// Returns true on success, logs and returns false on error.
/// Opens native file dialog and asks the user to choose a location in which to save a PNG file.
/// NOTE: assumes PNG file filter
bool FileDialog_new_png(char *out_path, size_t capacity);


#endif // FILE_DIALOG_H_
