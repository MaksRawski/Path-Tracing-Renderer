#ifndef STRATEGIES_H_
#define STRATEGIES_H_

#include "scene/bvh.h"

FindBestSplitFn FindBestSplitFn_longest_mid;
FindBestSplitFn FindBestSplitFn_SAH;

typedef enum {
  FindBestSplitFn_Variants_Naive,
  FindBestSplitFn_Variants_SAH,
  FindBestSplitFn_Variants__COUNT,
} FindBestSplitFn_Variants;

static FindBestSplitFn
    *FindBestSplitFn_Variants_get[FindBestSplitFn_Variants__COUNT] = {
        FindBestSplitFn_longest_mid, FindBestSplitFn_SAH};

static const char
    *FindBestSplitFn_Variants_names[FindBestSplitFn_Variants__COUNT] = {"Naive",
                                                                        "SAH"};

#endif // STRATEGIES_H_
