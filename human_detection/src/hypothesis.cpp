#include "hypothesis.h"
#include "poselet_detector.h"
#include "xml_utils.h"
#include <iostream>

using namespace std;
using namespace boost::gil;

template <typename InputStream, typename T>
InputStream& operator>>(InputStream& is, boost::gil::point2<T>& p) {
  is >> p.x >> p.y;
  return is;
}



hypothesis::hypothesis(rapidxml::xml_node<>* node, size_t num_keypoints)
 : _kp_distribs(num_keypoints) {
  read_xml_vector(get_xml_value(node->first_node("coords_sum")),
                  num_keypoints, _coords_sum);
  read_xml_vector(get_xml_value(node->first_node("coords_sum2")),
                  num_keypoints, _coords_sum2);
  read_xml_vector(get_xml_value(node->first_node("w_sum")),
                  num_keypoints, _w_sum);
  recompute_distributions();
}

void hypothesis::transform(const float_point& tr, float scale) {
  std::vector<gauss2d> kp_distribs(_kp_distribs);
  for (size_t i=0; i<_kp_distribs.size(); ++i) {
    kp_distribs[i]._mu.x=kp_distribs[i]._mu.x * scale + tr.x;
    kp_distribs[i]._mu.y=kp_distribs[i]._mu.y * scale + tr.y;
    kp_distribs[i]._sigma.x*=(scale*scale);
    kp_distribs[i]._sigma.y*=(scale*scale);
  }
  set_kp_distribs(kp_distribs);
}

void hypothesis::set_kp_distribs(const std::vector<gauss2d>& kp_distribs) {
  const float pvar  = config::get().HYPOTHESIS_PRIOR_VAR;
  const float pvarw = config::get().HYPOTHESIS_PRIOR_VARIANCE_WEIGHT;

  for (size_t i=0; i<_kp_distribs.size(); ++i) {
    float_point mu=kp_distribs[i]._mu;

    _coords_sum[i].x = mu.x*_w_sum[i].x;
    _coords_sum[i].y = mu.y*_w_sum[i].y;

    float_point sum1 = _w_sum[i] + float_point(pvarw,pvarw);
    _coords_sum2[i].x = (kp_distribs[i]._sigma.x * sum1.x
      + 2 * mu.x * _coords_sum[i].x - mu.x*mu.x*_w_sum[i].x - pvar*pvarw);
    _coords_sum2[i].y = (kp_distribs[i]._sigma.y * sum1.y
      + 2 * mu.y * _coords_sum[i].y - mu.y*mu.y*_w_sum[i].y - pvar*pvarw);
  }
  recompute_distributions();
}

// recompute the Gaussians after changing the sufficient statistics
void hypothesis::recompute_distributions() {
  _rect = float_bounds();
  const float pvar  = config::get().HYPOTHESIS_PRIOR_VAR;
  const float pvarw = config::get().HYPOTHESIS_PRIOR_VARIANCE_WEIGHT;
  for (size_t i=0; i<_kp_distribs.size(); ++i) {
    float_point& mu=_kp_distribs[i]._mu;
    if (_w_sum[i].x>0) {
      mu.x = _coords_sum[i].x / _w_sum[i].x;
      mu.y = _coords_sum[i].y / _w_sum[i].y;
      _rect.set_to_union(mu);
    } else {
      mu=float_point(0,0);
    }

    _kp_distribs[i]._sigma.x = max(0.0000001f,
       (_coords_sum2[i].x - 2*mu.x*_coords_sum[i].x
        + mu.x*mu.x*_w_sum[i].x + pvar*pvarw) / (_w_sum[i].x + pvarw));
    _kp_distribs[i]._sigma.y = max(0.0000001f,
       (_coords_sum2[i].y - 2*mu.y*_coords_sum[i].y
        + mu.y*mu.y*_w_sum[i].y + pvar*pvarw) / (_w_sum[i].y + pvarw));
  }
}

// return the symmetrized KL divergence
//#pragma optimize("",off)
double hypothesis::distance(const hypothesis& h) const {
  if (!h._rect.intersects(_rect))
    return std::numeric_limits<double>::infinity();

  double dist=0;
  double count=0;
  for (size_t i=0; i<_kp_distribs.size(); ++i) {
    if (_w_sum[i].x==0 || h._w_sum[i].x==0)
      continue;
    float_point meandiff_sqrd = (_kp_distribs[i]._mu - h._kp_distribs[i]._mu);
    meandiff_sqrd.x*=meandiff_sqrd.x;
    meandiff_sqrd.y*=meandiff_sqrd.y;

    dist += (_kp_distribs[i]._sigma.x + meandiff_sqrd.x)
      / h._kp_distribs[i]._sigma.x
      + (h._kp_distribs[i]._sigma.x + meandiff_sqrd.x)
      / _kp_distribs[i]._sigma.x - 2;
    dist += (_kp_distribs[i]._sigma.y + meandiff_sqrd.y)
      / h._kp_distribs[i]._sigma.y
      + (h._kp_distribs[i]._sigma.y + meandiff_sqrd.y)
      / _kp_distribs[i]._sigma.y - 2;
    count+=2;
  }
  return dist/count;
}
