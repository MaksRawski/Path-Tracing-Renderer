#include "window/resolution.h"

WindowResolution WindowResolution_new(unsigned int width, unsigned int height) {
  return (WindowResolution){width, height};
}

bool WindowResolution_eq(WindowResolution a, WindowResolution b) {
  return (a.width == b.width && a.height == b.height);
}
