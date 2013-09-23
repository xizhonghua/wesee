#pragma once

#include <vector>
#include "config.h"
#include "poselet.h"

class poselets_model {
private:
  std::vector<poselet> _poselets;
  std::vector<float>   _wts;
  double _cluster_thresh;
  std::string _object_type;

  struct dims_group {
    int_point _dims;            // dimensions in bins
    std::vector<size_t> _poselet_ids;
  };
  std::vector<dims_group> _dims_groups; // poselets grouped by dimension

  std::vector<float> _bounds_regress;
public:
  poselets_model() {}

  void init(const char* model_name);
  const char* object_type() const { return _object_type.c_str(); }
  size_t num_poselets() const { return _poselets.size(); }
  const poselet& operator[](size_t i) const { return _poselets[i]; }
  const std::vector<float>& bounds_regress() const { return _bounds_regress; }

  const std::vector<dims_group>& dims_groups() const { return _dims_groups; }
  double cluster_thresh() const { return _cluster_thresh; }
  float wts(size_t poselet_id) const { return _wts[poselet_id]; }
};
