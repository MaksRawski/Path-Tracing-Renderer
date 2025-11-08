#ifndef INPUTS_H_
#define INPUTS_H_

#include <GLFW/glfw3.h>

#define STEP_SIZE_PER_FRAME 0.05
#define CURSOR_SENSITIVITY 0.001

void cursor_callback(GLFWwindow *window, double xPos, double yPos);
void cursor_enter_callback(GLFWwindow *window, int entered);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

#endif // INPUTS_H_
