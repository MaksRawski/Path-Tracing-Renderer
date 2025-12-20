#ifndef SCALING_H_
#define SCALING_H_

typedef enum {
  OpenGLScalingMode_FIT_CENTER,
  OpenGLScalingMode_STRETCH,
  OpenGLScalingMode__COUNT,
} OpenGLScalingMode;

static const char *OpenGLScalingMode_str[OpenGLScalingMode__COUNT] = {
    "Fit", "Stretch"};

#endif // SCALING_H_
