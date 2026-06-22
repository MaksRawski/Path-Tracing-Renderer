#include "gui/file_browser.h"

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
#elif defined(__linux__)
#include "asserts.h"
// for popen and pclose
#ifndef __USE_POSIX2 
#define __USE_POSIX2 
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
bool GuiFileBrowser_open(char out_path[], size_t capacity) {
  if (system("zenity --version &> /dev/null") == 1)
    ERROR("zenity not installed!");
  FILE *fd = popen("zenity --file-selection", "r");
  if (fd == NULL)
    ERROR("Failed to create zenity process!");
  if (fgets(out_path, capacity, fd) == NULL)
    return false;
  else
    out_path[strcspn(out_path, "\n")] = 0;
  int status = pclose(fd);
  if (status == -1) {
    perror("pclose");
    return false;
  }
  return status == 0;
}
#endif
