#ifndef RESOLUTION_H_
#define RESOLUTION_H_
#include <stdbool.h>

typedef struct {
  unsigned int width, height;
} WindowResolution;

WindowResolution WindowResolution_new(unsigned int width, unsigned int height);
bool WindowResolution_eq(WindowResolution a, WindowResolution b);

#endif // RESOLUTION_H_
