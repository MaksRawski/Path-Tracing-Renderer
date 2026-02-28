#ifndef SCALING_H_
#define SCALING_H_

typedef enum {
  WindowScalingMode_FIT_CENTER,
  WindowScalingMode_STRETCH,
  WindowScalingMode__COUNT,
} WindowScalingMode;

static const char *WindowScalingMode_str[WindowScalingMode__COUNT] = {
    "Fit", "Stretch"};

#endif // SCALING_H_
