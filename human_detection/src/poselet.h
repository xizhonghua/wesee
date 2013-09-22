#pragma once
#include "config.h"
#include "hypothesis.h"
#include "classifier.h"
#include <vector>

class poselet {
public:
  poselet() {}

  poselet(rapidxml::xml_node<>* node, int num_poselets, int num_keypoints);

  poselet(const poselet& p) : _dims(p._dims), _classifier(p._classifier),
    _bigq_classifier(p._bigq_classifier), _obj_bounds(p._obj_bounds),
    _obj_bounds_var(p._obj_bounds_var), _hypothesis(p._hypothesis) {}

  poselet& operator=(const poselet& p) {
    if (&p!=this) {
      using namespace std;
      poselet tmp(p);
      swap(tmp,*this);
    }
    return *this;
  }
  bool operator==(const poselet& p) const {
    return _dims==p._dims && _classifier==p._classifier
    && _bigq_classifier==p._bigq_classifier
    &&  _obj_bounds==p._obj_bounds && _obj_bounds_var==p._obj_bounds_var; }
  bool operator!=(const poselet& p) const { return !(*this==p); }

  const int_point& dims() const { return _dims; }

  double inner_product(const float* hog) const {
    return _classifier.inner_product(hog);
  }
  double partial_inner_product(const float* hog, const int& start,
                               const int& width) const {
    return _classifier.partial_inner_product(hog, start, width); }
  double get_offset() const {return _classifier.get_offset(); }
  const std::vector<float>& get_weights() const {
    return _classifier.get_weights(); }
  double probability(double s) const { return _classifier.evaluate_sigmoid(s); }
  double bigq_probability(const float* context_features) const {
    return _bigq_classifier.evaluate(context_features); }
  const hypothesis& get_hypothesis() const { return _hypothesis; }
  const float_bounds& obj_bounds() const { return _obj_bounds; }
  const float_bounds& obj_bounds_var() const { return _obj_bounds_var; }
private:
  friend void swap(poselet&,poselet&);
  int_point _dims;
  linear_classifier<float> _classifier;
  linear_classifier<float> _bigq_classifier;
  float_bounds _obj_bounds, _obj_bounds_var;

  hypothesis _hypothesis;
};


inline void swap(poselet& p1, poselet& p2) {
  using namespace std;
  swap(p1._dims,p2._dims);
  swap(p1._classifier,p2._classifier);
  swap(p1._bigq_classifier,p2._bigq_classifier);
  swap(p1._obj_bounds,p2._obj_bounds);
  swap(p1._obj_bounds_var,p2._obj_bounds_var);
  swap(p1._hypothesis,p2._hypothesis);
}

