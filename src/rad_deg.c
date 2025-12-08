#include "rad_deg.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double rad_to_deg(double rad) { return rad * 180.0 / M_PI; }

double deg_to_rad(double deg) { return deg / 180.0 * M_PI; }
