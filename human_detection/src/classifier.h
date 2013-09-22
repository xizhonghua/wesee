#pragma once

#include "xml_utils.h"
#include <vector>
#include <sstream>
#include <boost/multi_array.hpp>
#include <boost/lexical_cast.hpp>

// fast inner product
float do_inner_product(const float* a, const float* b, size_t size);

// the sigmoid function f(x) = 1 / (1 + exp(a*x + b))
template <typename Real>
class sigmoid {
public:
  sigmoid() : _logit_a(Real(1)), _logit_b(0) {}
  sigmoid(Real logit_a, Real logit_b) : _logit_a(logit_a), _logit_b(logit_b) {}
  sigmoid(const sigmoid& s) :  _logit_a(s._logit_a), _logit_b(s._logit_b) {}
  sigmoid& operator=(const sigmoid& s) {
    sigmoid tmp(s); swap(tmp,*this); return *this;
  }

  friend void swap(sigmoid& a, sigmoid& b) {
    using std::swap; swap(a._logit_a, b._logit_a); swap(a._logit_b, b._logit_b);
  }
  bool operator==(const sigmoid& c) const {
    return _logit_a==c._logit_a && _logit_b==c._logit_b;
  }
  bool operator!=(const sigmoid& p) const { return !(*this==p); }

  Real evaluate(Real score) const {
    return Real(1.0)/(Real(1.0)+exp(score*_logit_a+_logit_b));
  }
private:
  Real _logit_a,_logit_b;
};

// linear classifier plus a logistic
template <typename Real>
class linear_classifier {
public:
  linear_classifier() {}
  linear_classifier(const linear_classifier& c)
    : _weights(c._weights), _sig(c._sig) {}
  explicit linear_classifier(rapidxml::xml_node<>* node);

  linear_classifier& operator=(const linear_classifier& c) {
    linear_classifier tmp(c);
    swap(tmp,*this);
    return *this;
  }
  friend void swap(linear_classifier& a, linear_classifier& b) {
    using std::swap;
    swap(a._weights, b._weights);
    swap(a._sig, b._sig);
  }
  bool operator==(const linear_classifier& c) const {
    return _sig==c._sig && _weights==c._weights; }
  bool operator!=(const linear_classifier& p) const { return !(*this==p); }

  double evaluate(const Real* features) const {
    return evaluate_sigmoid(inner_product(features));
  }

  Real inner_product(const Real* features) const {
    return do_inner_product(&_weights[0], features,
                            _weights.size()-1)+_weights.back();
  }
  double evaluate_sigmoid(double score) const { return _sig.evaluate(score); }
  Real partial_inner_product(const Real* features,
                             const int& start,
                             const int& width) const {
    return do_inner_product(&_weights[start], features, width);
  }

  double get_offset() const { return _weights.back(); }
  const std::vector<Real>& get_weights() const { return _weights; }
private:
  friend class poselet;
  std::vector<Real> _weights;
  sigmoid<double> _sig;
};


// polynomial classifier plus logistic
template <typename Real>
class poly_svm_classifier {
public:
  poly_svm_classifier() {}
  poly_svm_classifier(const poly_svm_classifier& c)
    : _coef0(c._coef0), _gamma(c._gamma), _degree(c._degree), _rho(c._rho),
    _sv(c._sv), _sv_coef(c._sv_coef), _sig(c._sig) {}
  explicit poly_svm_classifier(rapidxml::xml_node<>* node);

  poly_svm_classifier& operator=(const poly_svm_classifier& c) {
    poly_svm_classifier tmp(c);
    swap(tmp,*this);
    return *this;
  }
  friend void swap(poly_svm_classifier& a, poly_svm_classifier& b) {
    using std::swap;
    swap(a._coef0, b._coef0);
    swap(a._gamma, b._gamma);
    swap(a._degree, b._degree);
    swap(a._rho, b._rho);
    swap(a._sv, b._sv);
    swap(a._sv_coef, b._sv_coef);
    swap(a._sig, b._sig);
  }
  bool operator==(const poly_svm_classifier& c) const {
    return _coef0==c._coef0 && _gamma=c._gamma && _degree==c._degree &&
    _rho==c._rho && _sig==c._sig && _sv_coef==c._sv_coef && _sv==c._sv; }
  bool operator!=(const poly_svm_classifier& p) const { return !(*this==p); }

  Real evaluate(const Real* features) const;
private:
  Real _coef0,_gamma,_degree,_rho;
  boost::multi_array<Real,2> _sv;
  std::vector<Real> _sv_coef;
  sigmoid<Real> _sig;
};


template <typename Real>
poly_svm_classifier<Real>::poly_svm_classifier(rapidxml::xml_node<>* root) {
  int num_sv
    = boost::lexical_cast<int>(get_xml_value(root->first_attribute("num_sv")));
  int sv_len
    = boost::lexical_cast<int>(get_xml_value(root->first_attribute("sv_len")));
  _coef0
    = boost::lexical_cast<Real>(get_xml_value(root->first_attribute("coef0")));
  _gamma
    = boost::lexical_cast<Real>(get_xml_value(root->first_attribute("gamma")));
  _degree
    = boost::lexical_cast<Real>(get_xml_value(root->first_attribute("degree")));
  _rho= boost::lexical_cast<Real>(get_xml_value(root->first_attribute("rho")));

  Real logit_a,logit_b;
  std::stringstream lstr(get_xml_value(root->first_attribute("logit_coef")));
  lstr >> logit_a >> logit_b;
  _sig=sigmoid<Real>(logit_a,logit_b);

  _sv.resize(boost::extents[num_sv][sv_len]);
  _sv_coef.resize(num_sv);
  rapidxml::xml_node<> *node
    = root->first_node("support_vectors")->first_node("sv");
  assert(node);

  std::vector<Real> sv;
  for (int i=0; i<num_sv; ++i) {
    _sv_coef[i] =
      boost::lexical_cast<Real>(get_xml_value(node->first_attribute("coef")));
    read_xml_vector(get_xml_value(node), sv_len, sv);
    for (int j=0; j<sv_len; ++j)
      _sv[i][j]=sv[j];

    node=node->next_sibling("sv");
  }
  assert(!node);
}

template <typename Real>
Real poly_svm_classifier<Real>::evaluate(const Real* features) const {
  Real score=-_rho;
  size_t sv_len = _sv.shape()[1];
  for (size_t sv=0; sv<_sv_coef.size(); ++sv)
    score += _sv_coef[sv]
      * pow(_gamma*do_inner_product(features,&_sv[sv][0],sv_len),_degree);
  return _sig.evaluate(score);
}






template <typename Real>
linear_classifier<Real>::linear_classifier(rapidxml::xml_node<>* node) {
  std::stringstream str(get_xml_value(node));
  float w;
  str>>w;
  while (!str.eof()) {
    _weights.push_back(w);
    str>>w;
  }

  double logit_a,logit_b;
  std::stringstream lstr(get_xml_value(node->first_attribute("logit_coef")));
  lstr >> logit_a >> logit_b;
  _sig=sigmoid<double>(logit_a,logit_b);
}
