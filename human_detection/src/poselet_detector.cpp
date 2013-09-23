#include "poselet_detector.h"
#include "poselet_cluster.h"
#include "agglomerative_cluster.h"
//#include "xml_utils.h"
#include <algorithm>
#include <list>
#include <set>
#include <fstream>
#include <iostream>
#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost::gil;

void poselet_detector::init(const char* root, const char* cat_name, bool seq) {
  string model_file = string(root) + "/" + cat_name + ".xml";
  _model.init(model_file.c_str());
  _image2hits.init(&_model, root, seq);
}

hit poselet_detector::cluster2torso_hit(const poselet_hits_vector& hits,
                                        const vector<hypothesis>& hyps,
                                        const vector<size_t>& cluster) const {
  // Get the hips/shoulders coordinates
  const int kp_idx[]={1-1, 4-1, 7-1, 10-1};// L_Shoulder,R_Shoulder, L_Hip,R_Hip
  double_point torso[4];

  for (size_t j=0; j<4; ++j) {
    torso[j].x=torso[j].y=0;
    double_point weight_sum(0,0);
    for (size_t i=0; i<cluster.size(); ++i) {
      size_t hit_id=cluster[i];
      float_point var = hyps[hit_id].kp_sigma(kp_idx[j]);
      float_point mean = hyps[hit_id].kp_mean(kp_idx[j]);
      double_point wp(hits[hit_id].score()/var.x, hits[hit_id].score()/var.y);

      torso[j].x += mean.x * wp.x;
      torso[j].y += mean.y * wp.y;
      weight_sum+=wp;
    }
    torso[j].x/=weight_sum.x;
    torso[j].y/=weight_sum.y;
  }
  double torso_score = 0;
  for (size_t i=0; i<cluster.size(); ++i) {
    torso_score += hits[cluster[i]].score();
  }
  return hit(torso_score, keypoints2torso_bounds(torso));
}

struct hit_cluster {
  hit _hit;
  std::vector<size_t> _src_idx;
};


struct hit_overlap_dist_fn
    : public std::binary_function<float,hit_cluster,hit_cluster> {
  float operator()(const hit_cluster& h1, const hit_cluster& h2) const {
    return 1-float(bounds_overlap(h1._hit.bounds(),h2._hit.bounds()));
  }
};

struct hit_merge_fn : public std::binary_function<hit,hit,hit> {
  hit_cluster operator()(const hit_cluster& h1, const hit_cluster& h2) const {
    double score_sum=h1._hit.score() + h2._hit.score();
    hit_cluster ret;
    ret._hit = hit(score_sum,get_interpolated_bounds(
                     h1._hit.bounds(),float(h1._hit.score()/score_sum),
                     h2._hit.bounds(),float(h2._hit.score()/score_sum)));
    ret._src_idx=h1._src_idx;
    ret._src_idx.insert(ret._src_idx.end(),
                        h2._src_idx.begin(), h2._src_idx.end());
    return ret;
  }
};


// This can be sped up if necessary: hypotheses need not be initialized twice.
// The distance need not be computed twice
void poselet_detector::poselet_hits2bigq_poselet_hits(
    poselet_hits_vector& hits,
    vector<selection_t>& contributing_hits
  ) const {
  // instantiate the hypotheses (duplicated in poselet_hits2objects)
  vector<hypothesis> hyps(hits.size());
  for (size_t i=0; i<hits.size(); ++i) {
    float_point ctr(hits[i].min_pt().x + hits[i].dims().x/2,
                    hits[i].min_pt().y + hits[i].dims().y/2);
    hyps[i]=_model[hits[i].poselet_id()].get_hypothesis();
    hyps[i].transform(ctr, min(hits[i].dims().x, hits[i].dims().y));
  }

  vector<float> bigq_scores(hits.size());
  contributing_hits.clear();
  contributing_hits.resize(hits.size());

  vector<float> bigq_features(_model.num_poselets(), 0);
  for (size_t i=0; i<hits.size(); ++i) {
    std::fill(bigq_features.begin(), bigq_features.end(), 0.0f);
    for (size_t j=0; j<hits.size(); ++j) {
      if (hyps[i].distance(hyps[j]) < config::get().HYP_CLUSTER_THRESH) {
        if (bigq_features[hits[j].poselet_id()] < hits[j].score())
          bigq_features[hits[j].poselet_id()] = float(hits[j].score());
        contributing_hits[i].push_back(j);
      }
    }
    bigq_scores[i] =
      float(_model[hits[i].poselet_id()].bigq_probability(&bigq_features[0]));
  }

  for (size_t i=0; i<hits.size(); ++i)
    hits[i].set_score(bigq_scores[i]);
}

void poselet_detector::poselet_hits2objects(
  const poselet_hits_vector& hits,
  std::vector<object_hypothesis>& obj_hits,
  std::vector<selection_t>& contrib_hits,
  const int_point& img_dims) const {
  // instantiate the hypotheses
  vector<hypothesis> hyps(hits.size());
  for (size_t i=0; i<hits.size(); ++i) {
    float_point ctr(hits[i].min_pt().x + hits[i].dims().x/2,
                    hits[i].min_pt().y + hits[i].dims().y/2);
    hyps[i]=_model[hits[i].poselet_id()].get_hypothesis();
    hyps[i].transform(ctr, min(hits[i].dims().x, hits[i].dims().y));
  }

  // greedy cluster
  vector<selection_t> hit_clusters;
  cluster_poselet_hits(hits,hyps,_model.cluster_thresh(), hit_clusters);

  // Include the contributing small-qs in the clusters
  set<size_t> contrib_hits_in_cluster;
  vector<selection_t> extended_hit_clusters(hit_clusters.size());
  for (size_t c=0; c<hit_clusters.size(); ++c) {
    const selection_t& hit_cluster = hit_clusters[c];
    contrib_hits_in_cluster.clear();
    for (size_t i=0; i<hit_cluster.size(); ++i) {
      const selection_t& contrib_for_i = contrib_hits[hit_cluster[i]];
      contrib_hits_in_cluster.insert(contrib_for_i.begin(),
                                     contrib_for_i.end());
    }
    contrib_hits_in_cluster.insert(hit_cluster.begin(), hit_cluster.end());
    extended_hit_clusters[c].assign(contrib_hits_in_cluster.begin(),
                                    contrib_hits_in_cluster.end());
  }

  // Compute torso hits for each cluster
  vector<hit_cluster> torso_hits(hit_clusters.size());
  torso_hits.reserve(hit_clusters.size());
  for (size_t c=0; c<hit_clusters.size(); ++c) {
    torso_hits[c]._hit=cluster2torso_hit(hits,hyps,extended_hit_clusters[c]);
    torso_hits[c]._src_idx.push_back(c);
  }

  // Cluster them
  vector<hit_cluster> clustered_torso_hits;
  agglomerative_cluster(torso_hits.begin(), torso_hits.size(),
     hit_overlap_dist_fn(), hit_merge_fn(), config::get().CLUSTER_HITS_CUTOFF,
     back_inserter(clustered_torso_hits));

  size_t num_objects=clustered_torso_hits.size();

  // for each cluster find all the hits in it. Remove any duplicate hits,
  // or hits of the same poselet type and smaller score
  obj_hits.clear();
  //num_objects = 1;
  obj_hits.resize(num_objects);

  for (size_t c=0; c<num_objects; ++c) {
    poselet_hits_vector phv;
    const std::vector<size_t>& src_idx=clustered_torso_hits[c]._src_idx;
    for (size_t j=0; j<src_idx.size(); ++j) {
      const selection_t& ssrc_idx=hit_clusters[src_idx[j]];
      for (size_t k=0; k<ssrc_idx.size(); ++k)
        phv.push_back(hits[ssrc_idx[k]]);
    }
    obj_hits[c].init(phv, _model, img_dims);
  }
}

void save_object_n_poselet_hits(
  const std::vector<object_hypothesis>& object_hits,
  const std::string& filename) {
  poselet_hits_vector oph;
  for (size_t i=0; i<object_hits.size(); ++i) {
    // Create a poselet hit to represent the object hypothesis:
    // Overload the poselet ID field to store the number of hits
    // corresp. to the object hypothesis.
    poselet_hit obj(object_hits[i].poselet_hits().size(),
                    object_hits[i].score(), object_hits[i].bounds());
    const poselet_hits_vector& oh = object_hits[i].poselet_hits();
    oph.push_back(obj);
    oph.insert(oph.end(), oh.begin(), oh.end());
  }
  save_hits(oph, filename);
}
