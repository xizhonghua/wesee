#include "poselet_cluster.h"
#include <set>
#include "hypothesis.h"
#include "poselet_hit.h"

using namespace std;

struct poselet_score_fn : public unary_function<double,size_t> {
  explicit poselet_score_fn(const vector<poselet_hit>& hits) : _hits(hits) {}

  double operator()(size_t i) const { return _hits[i].score(); }
private:
  const vector<poselet_hit>& _hits;
};

struct poselet_cluster {
  vector<size_t> _hit_indices;
  set<size_t> _poselets;
};

// Models distance to cluster as mean KL-divergence distance to elements
// in cluster. If cluster has more than 5 elems, pics 5
// The cluster must not be empty
struct kl_divergence_to_subset_fn
    : public binary_function<double,size_t,const poselet_cluster&> {
  kl_divergence_to_subset_fn(const vector<hypothesis>& h,
      const vector<poselet_hit>& hits, size_t max_el2check)
      : _max_elems2check(max_el2check), _hyps(h), _hits(hits) {}

  double operator()(size_t i, const poselet_cluster& cluster) const {
    size_t n = cluster._hit_indices.size();
    assert(n>0);

    double dist=0;
    if (n <= _max_elems2check) {
      // check all
      double score_sum = 0;
      for (size_t c=0; c<n; ++c) {
        double score = _hits[cluster._hit_indices[c]].score();
        dist+=_hyps[i].distance(_hyps[cluster._hit_indices[c]]) * score;
        score_sum += score;
      }
      return dist/score_sum;
    }
    double step=(n-1)/double(_max_elems2check-1);
    double score_sum = 0;
    for (size_t j=0; j<_max_elems2check; ++j) {
      // check a subset of _max_elems2check equally spread
      size_t idx = cluster._hit_indices[size_t(j*step+0.5)];
      double score = _hits[idx].score();
      dist+=_hyps[i].distance(_hyps[idx]) * score;
      score_sum += score;
    }
    return dist/score_sum;
  }
private:
  size_t _max_elems2check;
  const vector<hypothesis>& _hyps;
  const vector<poselet_hit>& _hits;
};

struct append_if_unique : public binary_function<void,size_t,poselet_cluster&> {
  explicit append_if_unique(const vector<poselet_hit>& hits) : _hits(hits) {}

  bool operator()(size_t i, poselet_cluster& cluster) const {
    if (cluster._poselets.insert(_hits[i].poselet_id()).second) {
      try {
        cluster._hit_indices.push_back(i);
      } catch (...) {
        cluster._poselets.erase(_hits[i].poselet_id());
        throw;
      }
      return true;
    }
    return false;
  }
private:
  const vector<poselet_hit>& _hits;
};

void cluster_poselet_hits(
  const poselet_hits_vector& hits, const std::vector<hypothesis>& hyps,
  double dist_thresh, std::vector<std::vector<size_t> >& hit_clusters) {
  poselet_score_fn prob_fn(hits);
  kl_divergence_to_subset_fn dist_fn(hyps, hits,
                                     config::get().POSELET_CLUSTER_HITS2CHECK);
  append_if_unique append_fn(hits);
  std::vector<poselet_cluster> cluster_vec;

  greedy_cluster(prob_fn, dist_fn, append_fn, hits.size(), dist_thresh,
                 config::get().HYP_CLUSTER_MAXIMUM, cluster_vec);

  hit_clusters.resize(cluster_vec.size());
  for (size_t c=0; c<cluster_vec.size(); ++c)
    std::swap(hit_clusters[c],cluster_vec[c]._hit_indices);
}
