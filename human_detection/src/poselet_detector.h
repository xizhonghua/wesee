#pragma once
#include "hypothesis.h"
#include "poselet_hit.h"
#include "model.h"
#include "object_hypothesis.h"
#include <iostream>
#include <boost/multi_array.hpp>
#include "hits2features.h"
#include "image2hog.h"
#include <boost/timer.hpp>
#include <algorithm>

#include "image2hits.h"
typedef image2hits image2hits_type;

class poselet_detector {
public:
  poselet_detector() {}
  void init(const char* root_dir, const char* category_name, bool sequential);

  template <typename View> void detect_poselets(const View& v,
   poselet_hits_vector& hits_out) const {
    _image2hits.compute_hits(v, hits_out);
  }

  template <typename View> void detect_objects(const View& v, bool useBigQ,
    poselet_hits_vector& poselet_hits,
    std::vector<object_hypothesis>& object_hits) const;

  template <typename View> void get_features_of_hits(const View& v,
    const poselet_hits_vector& hits,
    std::vector<std::vector<float> >& features) const;

  const poselets_model& model() const { return _model; }

  friend void rawhits2objects(const poselet_detector* pd,
                              const poselet_hits_vector& rawhits,
                              poselet_hits_vector& poselet_hits,
                              std::vector<object_hypothesis>& object_hits);
private:
  typedef std::vector<size_t> selection_t;
  boost::multi_array<float,2> _poselet_svms;
  hog_features_generator _hg;
  poselets_model _model;
  mutable image2hits_type _image2hits;

  void poselet_hits2bigq_poselet_hits(
      poselet_hits_vector& hits, std::vector<selection_t>& contrib_hits) const;
  void poselet_hits2hit_clusters(const poselet_hits_vector& hits,
     std::vector<poselet_hits_vector>& clustered_hits) const;
  void poselet_hits2objects(const poselet_hits_vector& hits,
                            std::vector<object_hypothesis>& obj_hits,
                            std::vector<selection_t>& contrib_hits,
                            const int_point& dims) const;
  hit cluster2torso_hit(const poselet_hits_vector& hits,
     const std::vector<hypothesis>& hyps, const selection_t& cluster) const;
  void clipToImageBounds(poselet_hits_vector& hits, int w, int h) const;
};

void save_object_n_poselet_hits(
 const std::vector<object_hypothesis>& object_hits,
 const std::string& filename);

template <typename View>
void poselet_detector::detect_objects(
  const View& v, bool useBigQ, poselet_hits_vector& poselet_hits,
  std::vector<object_hypothesis>& object_hits) const {
  poselet_hits.clear();
  boost::timer t;

  //std::cout<<"detect_poselets"<<std::endl;
  detect_poselets(v,poselet_hits);

  std::vector<selection_t> contrib_hits;
  if (useBigQ) {
    poselet_hits2bigq_poselet_hits(poselet_hits, contrib_hits);
  }

  //std::cout<<"hits2objects"<<std::endl;
  poselet_hits2objects(poselet_hits, object_hits, contrib_hits,
      cast_point<int_point>(v.dimensions()));

//  std::cerr << "Computed poselet hits in " << t.elapsed()
//            << " sec." << std::endl;

  std::sort(object_hits.begin(), object_hits.end());

  if (_image2hits.save_file())
    save_object_n_poselet_hits(object_hits, _image2hits.save_file());
}


template <typename View>
void poselet_detector::get_features_of_hits(
  const View& v, const poselet_hits_vector& hits,
  std::vector<std::vector<float> >& features) const {
  hits2features h2f(_model,hits);
  h2f.compute_features(v, _image2hits.target_dims());
  using std::swap;
  swap(h2f._features, features);
}


