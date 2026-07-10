#ifndef GUI_FILE_BROWSER_H_
#define GUI_FILE_BROWSER_H_

#include "GLFW/glfw3.h"
#include <stdbool.h>
#include <stddef.h>

bool GuiFileBrowser_open(GLFWwindow *window, char out_path[], size_t capacity);

#endif // GUI_FILE_BROWSER_H_
