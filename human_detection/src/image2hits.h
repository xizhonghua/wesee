#pragma once
#include "config.h"
#include "hog_features.h"
#include "image_pyramid.h"
#include "model.h"
#include "poselet_hit.h"
#include <iostream>

class image2hits {
public:
  void init(const poselets_model* m, const char* foo=NULL, bool seq=true) {
    _model = m;
  }

  template <typename View>
  void compute_hits(const View& v, poselet_hits_vector& hits_out) {
    _hits.clear();
    generate_image_pyramid(v, target_dims(), *this);
    nonmax_suppress_hits(*_model, _hits, hits_out);
    std::cerr << "Total hits: " << _hits.size()
              << " after nonmax:" << hits_out.size() << std::endl;
  }
  const char* save_file() const { return NULL; }

  // called for each level of the image pyramid
  template <typename View>
    void operator()(const View& v, const int_point& imoffset, double scale) {
    hog_features hog;
    _hg.compute(v,imoffset,hog);

    poselet_hits_vector hits_of_scale;
    hog2poselet_hits(hog,hits_of_scale);

    for (size_t i=0; i<hits_of_scale.size(); ++i)
      hits_of_scale[i].transform(
          cast_point<float_point>(hog.offset()),
          float(1.0/scale));

    _hits.insert(_hits.end(), hits_of_scale.begin(), hits_of_scale.end());
  }

  static void nonmax_suppress_hits(const poselets_model& model,
                                   const poselet_hits_vector& hits_in,
                                   poselet_hits_vector& hits_out);
  void hog2poselet_hits(const hog_features& hog,
                        poselet_hits_vector& hits) const;
  int_point target_dims() const { return int_point(0,0); } // 0 means no target
protected:
  poselet_hits_vector _hits;
  hog_features_generator _hg;
  const poselets_model* _model;
};



