#pragma once

#include "util.h"

struct config {
  static const config& get() {
    static config gConfig;
    return gConfig;
  }

  void setToFast(bool fast) const;

  mutable int DETECTION_IMG_MIN_NUM_PIX;
  mutable int DETECTION_IMG_MAX_NUM_PIX;
  mutable double PYRAMID_SCALE_RATIO;
  const int_point IMAGE_MARGIN;
  const float HYPOTHESIS_PRIOR_VAR;
  const float HYPOTHESIS_PRIOR_VARIANCE_WEIGHT;
  const size_t POSELET_CLUSTER_HITS2CHECK;
  const size_t HYP_CLUSTER_MAXIMUM;
  const float TORSO_ASPECT_RATIO;
  const float CLUSTER_HITS_CUTOFF;
  const float HYP_CLUSTER_THRESH;
  const float CLUSTER_THRESH;
  const int RAND_GRANULARITY;
  const int_point TORSO_SIZE;
  const int_point TORSO_STEP;

private:
  config();
};
