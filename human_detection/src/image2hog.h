#pragma once
#include "hog_features.h"
#include "image_pyramid.h"

class image2hog {
 public:
  image2hog() {}

  template <typename View>
    void compute_hog(const View& v, const int_point& target_dims) {
    _hog_vector.clear();
    _scale_vector.clear();
    _footprint = 0;
    generate_image_pyramid(v, target_dims, *this);
  }

  // called for each level of the image pyramid
  template <typename View>
    void operator()(const View& v, const int_point& offset, double scale) {
    hog_features hog;
    _hg.compute(v,offset,hog);
    _hog_vector.push_back(hog);
    _scale_vector.push_back(scale);
    // collect memory footprint
    _footprint += _hg.footprint(cast_point<int_point>(v.dimensions()));

  }
  std::vector<hog_features> _hog_vector;
  std::vector<double> _scale_vector;
  int _footprint;
 private:
  hog_features_generator _hg;
};




