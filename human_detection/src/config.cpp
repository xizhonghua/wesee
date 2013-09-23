#include "config.h"
#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>

config::config()
: IMAGE_MARGIN(64*2,96*2),
    HYPOTHESIS_PRIOR_VAR(1), HYPOTHESIS_PRIOR_VARIANCE_WEIGHT(1),
    POSELET_CLUSTER_HITS2CHECK(5), HYP_CLUSTER_MAXIMUM(100),
    TORSO_ASPECT_RATIO(1.5),
    CLUSTER_HITS_CUTOFF(0.6f),
    HYP_CLUSTER_THRESH(5),
    CLUSTER_THRESH(5),
    RAND_GRANULARITY(100000),
    //TORSO_SIZE(64, 96),
    TORSO_SIZE(64*2, 96*2),
    TORSO_STEP(16, 24)
{
  setToFast(true);
}

void config::setToFast(bool fast) const {
  if (fast) {
    DETECTION_IMG_MIN_NUM_PIX = 500*500;
    DETECTION_IMG_MAX_NUM_PIX = 750*750;
    PYRAMID_SCALE_RATIO = 2;
  } else {
    DETECTION_IMG_MIN_NUM_PIX = 1000*1000;
    DETECTION_IMG_MAX_NUM_PIX = 1500*1500;
    PYRAMID_SCALE_RATIO = 1.1;
  }
}


