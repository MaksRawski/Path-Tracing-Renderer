#ifndef GUI_FILE_BROWSER_H_
#define GUI_FILE_BROWSER_H_

#include <stdbool.h>

void GuiFileBrowser_open(char *out_path);
void GuiFileBrowser_open_windows(char *out_path);
char *GuiFileBrowser_open_linux(void);

#endif // GUI_FILE_BROWSER_H_
