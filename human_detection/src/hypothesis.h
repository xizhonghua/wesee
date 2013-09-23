#pragma once
#include <vector>
#include "config.h"
#include "rapidxml.hpp"

struct gauss2d {
  float_point _mu, _sigma;
};

class hypothesis {
public:
  hypothesis() {}

  hypothesis(const hypothesis& h)
 : _coords_sum(h._coords_sum), _coords_sum2(h._coords_sum2),
    _w_sum(h._w_sum), _kp_distribs(h._kp_distribs), _rect(h._rect) {}

  hypothesis(rapidxml::xml_node<>* node, size_t num_keypoints);

  hypothesis& operator=(const hypothesis& h) {
    hypothesis tmp(h);
    swap(*this,tmp);
    return *this;
  }

  friend void swap(hypothesis& h1, hypothesis& h2) {
    using std::swap;
    swap(h1._coords_sum, h2._coords_sum);
    swap(h1._coords_sum2, h2._coords_sum2);
    swap(h1._w_sum, h2._w_sum);
    swap(h1._kp_distribs, h2._kp_distribs);
    swap(h1._rect, h2._rect);
  }
  double distance(const hypothesis& h) const;

  void transform(const float_point& tr, float scale);

  const float_point& kp_mean(size_t kp_id) const {
    return _kp_distribs[kp_id]._mu; }
  const float_point& kp_sigma(size_t kp_id) const {
    return _kp_distribs[kp_id]._sigma; }
private:
  void recompute_distributions();
  void set_kp_distribs(const std::vector<gauss2d>& kp_distribs);

  // sufficient statistics
  std::vector<float_point> _coords_sum,_coords_sum2,_w_sum;

  std::vector<gauss2d> _kp_distribs;
  float_bounds _rect;
};
