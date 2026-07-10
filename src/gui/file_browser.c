#include "gui/file_browser.h"
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
  if (fgets(out, out_size, fd) == NULL)
    return false;
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
// https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-common-dialog-boxes#opening-a-file
bool GuiFileBrowser_open(char out_path[], size_t capacity) {
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
bool GuiFileBrowser_open(char out_path[], size_t capacity) {
  if (system("which osascript &> /dev/null") == 1)
    ERROR("osascript not installed!");
  return read_output_of_cmd("osascript -e 'POSIX PATH of (choose file of type {\"///\", \"gltf\", \"glb\"})'", out_path, capacity);
}
#elif defined(__linux__)
bool GuiFileBrowser_open(char out_path[], size_t capacity) {
  if (system("zenity --version &> /dev/null") == 1)
    ERROR("zenity not installed!");
  // TODO: try to use kdialog or sth if zenity not installed
  return read_output_of_cmd("zenity --file-selection", out_path, capacity);
}
#endif // 
