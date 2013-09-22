#include "poselet.h"
#include "xml_utils.h"
#include "hog_features.h"

#include <fstream>

using namespace std;

poselet::poselet(rapidxml::xml_node<>* node, int num_poselets, int num_kp)
    : _hypothesis(node, num_kp) {
  assert(get_xml_name(node)=="poselet");
  {
    stringstream str(get_xml_value(node->first_attribute("dims")));
    str >> _dims.x >> _dims.y;
  }

  rapidxml::xml_node<>* obj_bounds=node->first_node("obj_bounds");
  {
    stringstream str(get_xml_value(obj_bounds->first_attribute("pos")));
    float_point dims;
    str >> _obj_bounds._min.x >> _obj_bounds._min.y >> dims.x >> dims.y;
    _obj_bounds._max =  _obj_bounds._min + dims;
  }
  {
    stringstream str(get_xml_value(obj_bounds->first_attribute("var")));
    str >> _obj_bounds_var._min.x >> _obj_bounds_var._min.y
        >> _obj_bounds_var._max.x >> _obj_bounds_var._max.y;
  }

  if (node->first_node("bigq_weights")!=NULL)
    _bigq_classifier = linear_classifier<float>(
      node->first_node("bigq_weights"));
  _classifier = linear_classifier<float>(node->first_node("svm_weights"));

  int_point cell_dims(_dims.x/8-1,_dims.y/8-1);
  //    int hog_size = cell_dims.x*cell_dims.y*36;

  // swap row/column for weights
  int bin_size=hog_config::get().bin_size();
  assert(int(_classifier._weights.size()) ==
         1 + bin_size*cell_dims.x*cell_dims.y);
  vector<float> swapped_weights=_classifier._weights;
  for (int y=0; y<cell_dims.y; ++y)
    for (int x=0; x<cell_dims.x; ++x) {
      const float* src=&     swapped_weights[0] + (x*cell_dims.y + y)*bin_size;
      float* dst=&_classifier._weights[0] + (y*cell_dims.x + x)*bin_size;
      copy(src, src+bin_size, dst);
    }
}



