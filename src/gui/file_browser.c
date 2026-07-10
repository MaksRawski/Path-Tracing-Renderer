#include "gui/file_browser.h"
#include "asserts.h"
#include "nfd.h"
#include "nfd_glfw3.h"

#include <stdlib.h>
#include <string.h>

static bool NFD_INIT = false;

bool GuiFileBrowser_open(GLFWwindow *window, char out_path[], size_t capacity) {
  if (!NFD_INIT) {
    if (NFD_Init() == NFD_ERROR)
      ERROR("NFD_Init errored");
    atexit(NFD_Quit);
    NFD_SetDisplayPropertiesFromGLFW();
    NFD_INIT = true;
  }

  const nfdu8filteritem_t filters[1] = {{"glTF", "gltf,glb"}};
  nfdopendialogu8args_t args = {0};
  args.filterList = filters;
  args.filterCount = 1;
  NFD_GetNativeWindowFromGLFWWindow(window, &args.parentWindow);
  nfdu8char_t *outPath;

  const nfdresult_t dialog_result = NFD_OpenDialogU8_With(&outPath, &args);
  bool res = false;

  switch (dialog_result) {
  case NFD_OKAY:
    strncpy(out_path, outPath, capacity);
    NFD_FreePathU8(outPath);
    res = true;
    break;
  case NFD_CANCEL:
    NFD_FreePathU8(outPath);
    break;
  case NFD_ERROR:
    NFD_FreePathU8(outPath);
    ERROR_FMT("(NFD): %s", NFD_GetError());
    break;
  }

  return res;
}

/* #ifdef _WIN32 */
/* #include <stdio.h> */
/* #include <windows.h> */
/* //
 * https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-common-dialog-boxes#opening-a-file
 */
/* bool GuiFileBrowser_open(char out_path[], size_t capacity) { */
/*   OPENFILENAME ofn; */
/*   char szFile[260]; */

/*   ZeroMemory(&ofn, sizeof(ofn)); */
/*   ofn.lStructSize = sizeof(ofn); */
/*   ofn.hwndOwner = NULL; */
/*   ofn.lpstrFile = szFile; */
/*   ofn.lpstrFile[0] = '\0'; */
/*   ofn.nMaxFile = sizeof(szFile); */
/*   ofn.lpstrFilter = "All\0*.*\0glTF\0*.gltf;*.glb\0"; */
/*   ofn.nFilterIndex = 1; */
/*   ofn.lpstrFileTitle = NULL; */
/*   ofn.nMaxFileTitle = 0; */
/*   ofn.lpstrInitialDir = NULL; */
/*   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; */

/*   if (GetOpenFileName(&ofn) == TRUE) { */
/*     snprintf(out_path, capacity, "%s", ofn.lpstrFile); */
/*     return true; */
/*   } else { */
/*     return false; */
/*   } */
/* } */
/* #elif defined(__linux__) */
/* #include "asserts.h" */
/* // for popen and pclose */
/* #ifndef __USE_POSIX2  */
/* #define __USE_POSIX2  */
/* #endif */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <stdlib.h> */
/* bool GuiFileBrowser_open(char out_path[], size_t capacity) { */
/*   if (system("zenity --version &> /dev/null") == 1) */
/*     ERROR("zenity not installed!"); */
/*   FILE *fd = popen("zenity --file-selection", "r"); */
/*   if (fd == NULL) */
/*     ERROR("Failed to create zenity process!"); */
/*   if (fgets(out_path, capacity, fd) == NULL) */
/*     return false; */
/*   else */
/*     out_path[strcspn(out_path, "\n")] = 0; */
/*   int status = pclose(fd); */
/*   if (status == -1) { */
/*     perror("pclose"); */
/*     return false; */
/*   } */
/*   return status == 0; */
/* } */
/* #endif */
