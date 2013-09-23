#pragma once

#include "classifier.h"

class attribute_classifier {
public:
  attribute_classifier() {}
  explicit attribute_classifier(rapidxml::xml_node<>* node);

  double evaluate(const poselet_hits_vector& poselet_hits,
                  const std::vector<float>& attr_features) const;
private:
  std::vector<linear_classifier> _level1_classifiers;   // for each poselet type
  linear_classifier _level2_classifier;
};

attribute_classifier::attribute_classifier(rapidxml::xml_node<>* node) {

}
double attribute_classifier::classify_attribute(
  const poselet_hits_vector& poselet_hits,
  const std::vector<float>& attr_features) const {
  // compute level1 score for each poselet hit and each attribute
  std::vector<float> level1_scores(num_poselets,0);
  for (int p=0; p<poselet_hits.size(); ++p)
    level1_scores[p] = _level1_classifiers[poselet_hits[p].poselet_id()]
      .evaluate(attr_features[p]);

  return _level2_classifier.evaluate(level1_scores);
}


class attributes_classifier {
public:
  attributes_classifier() {}
  void init(const char* model_file);

  typedef boost::gil::rgb8_view_t img_view_t;
  void classify_attributes(const img_view_t& v, const object_hypothesis& hyp,
                           std::vector<float>& attr_results) const;
private:
  typedef std::vector<float> attr_feature_t;

  void compute_attribute_features(const img_view_t& v, const poselet_hit& ph,
                                  attr_feature_t& feature) const;
  float evaluate_level1_classifier(const attr_feature_t& feature, int attribute,
                                   int poselet_id) const;

  // skin classifier
  // skin mask for hands + legs of each poselet type

  std::vector<attribute_classifier> _attribute_classifiers;
  std::vector<poly_svm_classifier> _context_classifiers;
};






void attribute_classifier::classify_attributes(
  const img_view_t& v,
  const object_hypothesis& hyp,
  std::vector<float>& attr_results) const {
  // extract features for each poselet hit
  std::vector<attr_feature_t> attr_features(hyp.poselet_hits().size());
  for (size_t i=0; i<attr_features.size(); ++i)
    compute_attribute_features(v, hyp.poselet_hits()[i], attr_features[i]);

  std::vector<float> level2_scores(num_attributes,0);
  for (int a=0; a<num_attributes; ++a)
    level2_scores[a] = _attribute_classifiers[a].evaluate(hyp.poselet_hits(),
                                                          attr_features);

  attr_results.resize(num_attributes);
  for (int a=0; a<num_attributes; ++a)
    attr_results[a] = _attribute_classifiers[a].evaluate_level3(attr_results);
}


void attribute_classifier::compute_attribute_features(
  const img_view_t& v, const poselet_hit& ph,  attr_feature_t& feature) const {
        // extract image patch
        // compute color histogram in HSV
        // compute pyramid HOG
        // compute skin values
}

