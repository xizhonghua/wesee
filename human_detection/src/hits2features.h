#pragma once
#include "model.h"
#include "image_pyramid.h"
#include "hog_features.h"

static void error_if(bool cond) {
  if (cond)
    throw;
}
// Given a set of hits + image returns the features that generated these hits
class hits2features {
public:
 hits2features(const poselets_model& model, const poselet_hits_vector& hits)
   : _hits(hits), _model(model) {}

  template <typename View>
    void compute_features(const View& v, const int_point& target_dims) {
    _norm_width.resize(_hits.size());
    for (size_t i=0; i<_hits.size(); ++i)
      _norm_width[i]=_model[_hits[i].poselet_id()].dims().x;
    _features.resize(_hits.size());
    generate_image_pyramid(v, target_dims, *this);

    for (size_t i=0; i<_hits.size(); ++i) {
      error_if(_features[i].empty());
      const poselet& ps=_model[_hits[i].poselet_id()];
      double score=ps.probability(ps.inner_product(&_features[i][0]));
      error_if(fabs(score-_hits[i].score())>0.001);
    }
  }

  // called for each level of the image pyramid
  template <typename View>
    void operator()(const View& v, const int_point& imoffset, double scale) {
    // find the indices of the hits that were active at this scale
    std::vector<size_t> hits_at_this_scale;
    for (size_t i=0; i<_hits.size(); ++i)
      if (fabs(_norm_width[i] - _hits[i].bounds().width()*scale)<0.01) {
        error_if(!_features[i].empty());
        hits_at_this_scale.push_back(i);
      }

    if (hits_at_this_scale.empty())
      return;

    hog_features hog;
    _hg.compute(v,imoffset,hog);

    for (size_t i=0; i<hits_at_this_scale.size(); ++i) {
      size_t j=hits_at_this_scale[i];
      int_point template_dims = _model[_hits[j].poselet_id()].dims()
        / hog_config::get().pix_per_bin() - int_point(1,1);
      float_point fp(float(_hits[j].bounds()._min.x * scale - hog.offset().x)
                     / hog_config::get().pix_per_bin(),
                     float(_hits[j].bounds()._min.y * scale - hog.offset().y)
                     / hog_config::get().pix_per_bin());
      //      assert(fabs(fp.x-int(fp.x))<0.0001 && fabs(fp.y-int(fp.y))<0.0001);
      int_point p=intround(fp);
      _features[j].resize(hog.bin_size()*template_dims.x*template_dims.y);
      hog.get_feature(p,template_dims,_features[j]);
    }
  }

  std::vector<std::vector<float> > _features;
private:
  std::vector<int> _norm_width;
  poselet_hits_vector _hits;
  hog_features_generator _hg;
  const poselets_model& _model;
};

