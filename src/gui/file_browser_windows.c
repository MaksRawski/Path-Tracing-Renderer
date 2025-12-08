#include "gui/file_browser.h"

/* #include "windows.h" */

/* // TODO: link with "comdlg32.lib" for windows */
/* #pragma comment(lib, "comdlg32.lib") */

/* void GuiFileBrowser_open_windows(char *out_path) { */
/*   // 1. Structure to store dialog box configuration */
/*     OPENFILENAME ofn;        */
    
/*     // 2. Buffer to store the selected file path */
/*     char szFile[260];       // MAX_PATH is usually 260 on Windows */

/*     // 3. Initialize the OPENFILENAME structure (Zero out memory) */
/*     ZeroMemory(&ofn, sizeof(ofn)); */
/*     ofn.lStructSize = sizeof(ofn); */
/*     ofn.hwndOwner = NULL;  // If you have a window handle, put it here */
/*     ofn.lpstrFile = szFile; */
    
/*     // Set the first character to null so the dialog doesn't show a "default" filename */
/*     ofn.lpstrFile[0] = '\0'; */
    
/*     ofn.nMaxFile = sizeof(szFile); */

/*     // 4. Set the filter for .txt files */
/*     // Format: "Display Name\0Pattern\0Display Name 2\0Pattern 2\0\0" */
/*     ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0"; */
/*     ofn.nFilterIndex = 1; */
/*     ofn.lpstrFileTitle = NULL; */
/*     ofn.nMaxFileTitle = 0; */
/*     ofn.lpstrInitialDir = NULL; // Default logic for initial directory */
/*     ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; */

/*     // 5. Display the Open File Dialog */
/*     //  */
/*     if (GetOpenFileName(&ofn) == TRUE) { */
/*         printf("Success! You chose: %s\n", ofn.lpstrFile); */
/*     } else { */
/*         printf("Dialog cancelled or an error occurred.\n"); */
/*     } */
/* } */
