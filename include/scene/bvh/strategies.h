#ifndef STRATEGIES_H_
#define STRATEGIES_H_

#include "scene/bvh.h"

FindBestSplitFn FindBestSplitFn_longest_mid;
FindBestSplitFn FindBestSplitFn_SAH;

typedef enum {
  BVHStrategy_Naive,
  BVHStrategy_SAH,
  BVHStrategy__COUNT,
} BVHStrategy;

static FindBestSplitFn *BVHStrategy_get[BVHStrategy__COUNT] = {
    FindBestSplitFn_longest_mid, FindBestSplitFn_SAH};

static const char *BVHStrategy_str[BVHStrategy__COUNT] = {"Naive", "SAH"};

#endif // STRATEGIES_H_
