#ifndef YAWPITCH_H_
#define YAWPITCH_H_
#include "vec3d.h"

typedef struct {
  double yaw, pitch;
} YawPitch;
YawPitch YawPitch_new(double yaw, double pitch);

typedef struct {
  Vec3d pos, dir;
} PosDir;
PosDir PosDir_new(Vec3d pos, Vec3d dir);

YawPitch YawPitch_from_dir(Vec3d dir);
Vec3d YawPitch_to_dir(YawPitch yp);


#endif // YAWPITCH_H_
