#pragma once

#include <vector>
#include <limits>
//#pragma optimize("",off)

////////////////////////////////////////////////////////////////////////////////
///
///  Performs greedy clustering of elements. Each element has probability and
//   there is a scalar distance between each pair of elements.
///  The elements must come sorted by decreasing probability.
///  Clustering works as follows: consider every element starting from the most
//   probable one. Compute its distance to the existing clusters.
///  If it is less than the distance threshold, assign it to the closest cluster
//   Otherwise it forms a new cluster.
///
////////////////////////////////////////////////////////////////////////////////

template <
  typename ProbFn, // double prob_fn(size_t i)  -- returns the probability of
                   // the i'th element. Elems with larger index
                   // must have lower/equal probability
  typename DistFn, // double dist_fn(size_t i, const Cluster& cluster)
                   // -- returns the distance between element i and the cluster
  typename AppendFn, // void append(size_t i, Cluster& cluster)
                   // -- attempts to append element i to the cluster
  typename Cluster>
  void greedy_cluster(const ProbFn& prob_fn, const DistFn& dist_fn,
                      const AppendFn& append_fn, std::size_t num_elems,
                      double dist_thresh, std::size_t max_clusters,
                      std::vector<Cluster>& clusters) {
  clusters.clear();
  for (std::size_t i=0; i<num_elems; ++i) {
    // find distance of element i to the closest cluster
    double closest_dist = std::numeric_limits<double>::infinity();
    int closest_cluster=-1;
    for (std::size_t c=0; c<clusters.size(); ++c) {
      double dist = dist_fn(i,clusters[c]);
      if (dist<closest_dist) {
        closest_dist = dist;
        closest_cluster=int(c);
      }
    }

    // append to the closest cluster or start a new one
    if (closest_dist<dist_thresh) {
      append_fn(i,clusters[closest_cluster]);
    } else if (clusters.size()<max_clusters) {
      clusters.push_back(Cluster());
      append_fn(i,clusters.back());
    }
  }
}

class poselet_hit;
class hypothesis;
void cluster_poselet_hits(
  const std::vector<poselet_hit>& hits, const std::vector<hypothesis>& hyps,
  double dist_thresh, std::vector<std::vector<std::size_t> >& clustered_hits);
