#include "object_hypothesis.h"
#include "model.h"
#include <iostream>
using namespace std;

// Constructs object hypothesis from a set of clustered poselet hits.
// Computes the score and bounds
void object_hypothesis::init(const poselet_hits_vector& poselet_hits,
                             const poselets_model& model,
                             const int_point& img_dims) {
  _poselet_hits = poselet_hits;
  _hit = hit(compute_score(model), compute_bounds(model, img_dims));
}


double object_hypothesis::compute_score(const poselets_model& model) const {
  double score=0;
  vector<float> pav(model.num_poselets(), 0);
  for (size_t i=0; i<_poselet_hits.size(); ++i) {
    int pid = _poselet_hits[i].poselet_id();
    pav[pid] = std::max<float>(pav[pid], _poselet_hits[i].score());
  }
  for (size_t i=0; i<pav.size(); ++i) {
    score += pav[i] * model.wts(i);
  }
  return score;
}

float_bounds predict_bounds(const poselet_hit& hit, const poselet& poselet) {
  float scale=std::min(hit.bounds().height(),hit.bounds().width());
  double_point image2poselet_ctr=hit.bounds().center();
  float_bounds scaled_bounds = poselet.obj_bounds()*scale;
  double_point poselet2bounds_ctr=scaled_bounds.center();

  float_point image2bounds_ctr
    = cast_point<float_point>(image2poselet_ctr+poselet2bounds_ctr);
  return float_bounds(image2bounds_ctr-scaled_bounds.dimensions()/2,
                      scaled_bounds.dimensions());
}

float_bounds object_hypothesis::compute_bounds(const poselets_model& model,
                                               const int_point& img_dims)
const {
  float_bounds pb(0,0,0,0);
  float_bounds weight_sum=pb;
  for (size_t i=0; i<_poselet_hits.size(); ++i) {
    const poselet_hit& hit=_poselet_hits[i];
    float_bounds pred=predict_bounds(hit,model[hit.poselet_id()]);

    for (size_t j=0; j<4; ++j) {
      float w=float(hit.score()) / (model[hit.poselet_id()].obj_bounds_var()[j]
                                    * hit.bounds().width());
      weight_sum[j]+=w;
      pb[j]+=pred[j]*w;
    }
  }

  for (size_t j=0; j<4; ++j)
    pb[j]/=weight_sum[j];

  // Apply bounds regression
  const std::vector<float>& br = model.bounds_regress();

  float_bounds pred_bounds = pb;
  pred_bounds[0] = pb[0]*br[0] + pb[1]*br[4] + pb[2]*br[8] + pb[3]*br[12];
  pred_bounds[1] = pb[0]*br[1] + pb[1]*br[5] + pb[2]*br[9] + pb[3]*br[13];
  pred_bounds[2] = pb[0]*br[2] + pb[1]*br[6] + pb[2]*br[10] + pb[3]*br[14];
  pred_bounds[3] = pb[0]*br[3] + pb[1]*br[7] + pb[2]*br[11] + pb[3]*br[15];

  bool CLIP_TO_IMG_BOUNDS = true;
  if (CLIP_TO_IMG_BOUNDS) {
    pred_bounds.set_to_intersection(float_bounds(1,1,img_dims.x,img_dims.y));
  }

  return pred_bounds;
}
