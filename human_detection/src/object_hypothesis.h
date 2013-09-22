#pragma once

#include "poselet_hit.h"
class poselets_model;

class object_hypothesis {
public:
  object_hypothesis() {}
  void init(const poselet_hits_vector& poselet_hits,
    const poselets_model& m, const int_point& img_dims);

  double score() const { return _hit.score(); }
  const float_bounds& bounds() const { return _hit.bounds(); }
  const poselet_hits_vector& poselet_hits() const { return _poselet_hits; }

  friend bool operator<(const object_hypothesis& a, const object_hypothesis& b)
  {
    return a.score() > b.score();
  }

  template <typename View, typename Color>
    void draw(const View& v, const Color& c, int thickness=1) const {
    _hit.draw(v,c,thickness);
  }
private:
  double compute_score(const poselets_model& model) const;
  float_bounds compute_bounds(const poselets_model& model,
                              const int_point& img_dims) const;

  hit _hit;                             // bounds and score
  poselet_hits_vector _poselet_hits;    // the clustered hits
};

