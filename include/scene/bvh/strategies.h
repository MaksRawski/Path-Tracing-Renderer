#ifndef STRATEGIES_H_
#define STRATEGIES_H_

#include "scene/bvh.h"

FindBestSplitFn FindBestSplitFn_midpoint;
FindBestSplitFn FindBestSplitFn_SAH;

typedef enum {
  BVHStrategy_Midpoint,
  BVHStrategy_SAH,
  BVHStrategy__COUNT,
} BVHStrategy;

static FindBestSplitFn *BVHStrategy_get[BVHStrategy__COUNT] = {
    FindBestSplitFn_midpoint, FindBestSplitFn_SAH};

static const char *BVHStrategy_str[BVHStrategy__COUNT] = {"Midpoint", "SAH"};

#endif // STRATEGIES_H_
