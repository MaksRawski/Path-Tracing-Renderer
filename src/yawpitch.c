#include "yawpitch.h"

#include "asserts.h"
#include "epsilon.h"
#include "vec3d.h"
#include <math.h>

#define UNUSED (void)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Vec3d YawPitch_to_dir(YawPitch yp) {
  // yaw is expected to be in range [0, 2*PI] but it'd be ok if it wasn't
  ASSERTQ_CONDF(yp.yaw_rad >= 0 && yp.yaw_rad <= 2 * M_PI, yp.yaw_rad);
  // this function explodes if pitch is out of exclusive range (-PI/2, PI/2)
  ASSERTQ_CONDF(yp.pitch_rad > -M_PI / 2.0 && yp.pitch_rad < M_PI / 2.0, yp.pitch_rad);

  // we can imagine (x, z) of dir as being a point on a unit circle
  // then as we want yp.yaw = 0 to correspond to x = 0 and z = -1
  // we shift all the points by 90 deg clockwise so that angle 0 is now at the
  // bottom of the circle and so we get:
  //  a = yp.yaw - PI / 2
  // and since by definition
  //  x = cos(a) and z = sin(a)
  // we can simplify
  //  x = cos(yp.yaw - PI / 2) =  sin(yp.yaw)
  //  z = sin(yp.yaw - PI / 2) = -cos(yp.yaw)

  return Vec3d_new(sin(yp.yaw_rad), tan(yp.pitch_rad), -cos(yp.yaw_rad));
}

YawPitch YawPitch_from_dir(Vec3d dir) {
  // as described above:
  //  x =  sin yaw
  //  z = -cos yaw
  // since the point described by these coordinates must lie on the unit circle,
  ASSERTQ_CONDF(-1.0 <= dir.x && dir.x <= 1.0, dir.x);
  ASSERTQ_CONDF(-1.0 <= dir.z && dir.z <= 1.0, dir.z);
  // knowing z allows us to already derive the absolute value of an angle,
  // then we only need to check the sign of x to know on which half is the
  // point.
  //
  // A Z
  // |
  // 1      _____
  // |     /     \
  // 0    |       | <-- yaw = PI / 2
  // |     \     /
  // -1     -----
  // |        ^
  // |        |\__ yaw = 0
  // |        |
  // |--------0----> X

  double yaw = acos(-dir.z); // [0, PI]
  // if x < 0, then we're on the left half of the circle,
  // then yaw has to be calculated by going clockwise
  if (dir.x < -EPSILON)
    yaw = 2 * M_PI - yaw; // [PI, 2PI]

  // yaw must be in [0, 2PI]
  ASSERTQ_CONDF(yaw > -EPSILON && yaw < 2.0 * M_PI + EPSILON, yaw);

  double pitch = atan(dir.y); // [-PI / 2, PI / 2]
  ASSERTQ_CONDF(pitch > (-M_PI / 2.0) && pitch < (M_PI / 2.0), pitch);

  return (YawPitch){.yaw_rad = yaw, .pitch_rad = pitch};
}

PosDir PosDir_new(Vec3d pos, Vec3d dir) { return (PosDir){pos, dir}; }

YawPitch YawPitch_new(double yaw, double pitch) {
  // TODO: normalize
  return (YawPitch){yaw, pitch};
}
