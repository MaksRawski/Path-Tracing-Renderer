#include "asserts.h"

#if defined(__linux__) || defined(__APPLE__)
// NOTE: for popen and pclose
#ifndef __USE_POSIX2
#define __USE_POSIX2
#endif // __USE_POSIX2
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool read_output_of_cmd(const char *cmd, char *out, size_t out_size) {
  FILE *fd = popen(cmd, "r");
  if (fd == NULL)
    ERROR("Failed to create a process!");
  if (fgets(out, out_size, fd) == NULL) {
    // NOTE: this is not a irrecoverable error so we just log it
    fprintf(stderr, RED("ERROR:") "fgets of popen(\"%s\") failed, out_size = %lu\n", cmd, out_size);
    return false;
  }
  else
    out[strcspn(out, "\n")] = 0;
  int status = pclose(fd);
  if (status == -1) {
    perror("pclose");
    return false;
  }
  return status == 0;
}
#endif // defined(__linux__) || defined(__APPLE__)


#ifdef _WIN32
#include <stdio.h>
#include <windows.h>
bool FileDialog_is_available(void) {
  // TODO: check if it actually is always available
  return true;
}
// https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-common-dialog-boxes#opening-a-file
bool FileDialog_open_gltf(char out_path[], size_t capacity) {
  OPENFILENAME ofn;
  char szFile[260];

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All\0*.*\0glTF\0*.gltf;*.glb\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileName(&ofn) == TRUE) {
    snprintf(out_path, capacity, "%s", ofn.lpstrFile);
    return true;
  } else {
    return false;
  }
}
#elif defined(__APPLE__)
bool FileDialog_is_available(void) {
  return (system("which osascript &> /dev/null") == 0);
}
bool FileDialog_open_gltf(char out_path[], size_t capacity) {
  return read_output_of_cmd("osascript -e 'POSIX PATH of (choose file of type {\"///\", \"gltf\", \"glb\"})'", out_path, capacity);
}
#elif defined(__linux__)
  // TODO: try to use kdialog or sth if zenity not installed,
  // remember to update FileDialog_not_available_msg afterwards
bool FileDialog_is_available(void) {
  return (system("zenity --version &> /dev/null") == 0);
}
bool FileDialog_open_gltf(char out_path[], size_t capacity) {
  return read_output_of_cmd("zenity --file-selection --file-filter='glTF(*.gltf, *.glb) | *.gltf *.glb'", out_path, capacity);
}
#endif // 
