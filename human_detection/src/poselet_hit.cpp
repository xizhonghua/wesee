#include "poselet_hit.h"
#include "hypothesis.h"
#include "xml_utils.h"
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace std;

void hit::init(rapidxml::xml_node<>* node) {
  _score = boost::lexical_cast<double>(
    get_xml_value(node->first_attribute("score")));

  stringstream str(get_xml_value(node->first_attribute("bounds")));
  str >> _bounds;
}

void poselet_hit::init(rapidxml::xml_node<>* node) {
  hit::init(node);
  _poselet_id = boost::lexical_cast<int>(
    get_xml_value(node->first_attribute("poselet_id")));
}

float_bounds poselet_hit::get_torso_bounds(hypothesis& hyps) const {
  // transform the torsos according to the location
  // & scale of the poselet hit
  const float_bounds& hb = bounds();
  float_point ctr(hb._min.x + hb.dimensions().x/2,
                  hb._min.y + hb.dimensions().y/2);
  float torso_scale = std::min(hb.dimensions().x,
                               hb.dimensions().y);
  hyps.transform(ctr, torso_scale);
  // from the 4 key points compute torso location
  const int kp_idx[]={1-1, 4-1, 7-1, 10-1};
  double_point torso[4];
  for(size_t j=0; j<4; ++j)
    torso[j] = cast_point<double_point>(hyps.kp_mean(kp_idx[j]));
  return keypoints2torso_bounds(torso);
}

ostream& operator<<(ostream& os, const hit& hit) {
  os << "   <hit score=\""<<hit.score()
     << "\" bounds=\""<<hit.bounds()<<"\" />"<<endl;
  return os;
}

ostream& operator<<(ostream& os, const poselet_hit& hit) {
  os << "   <poselet_hit score=\""<<hit.score()<<"\" bounds=\""
     <<hit.bounds()<<"\" poselet_id=\""<<hit.poselet_id()<<"\" />"<<endl;
  return os;
}

void save_hits(const poselet_hits_vector& hits,
               const std::string& filename) {
  std::ofstream fout(filename.c_str(), std::ios::out);
  fout << hits.size() << " ";
  for (size_t i = 0; i <hits.size(); ++i) {
    const float_bounds& b = hits[i].bounds();
    fout << hits[i].poselet_id() << " " << hits[i].score() << " " << b._min.x
         <<  " " << b._min.y << " " << b._max.x << " " << b._max.y << " ";
  }
  fout.close();
}

void save_poselet_hits(const poselet_hits_vector& hits,
                                         const char* filename)  {
  ofstream os(filename);
  os << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
  os << "<poselet_hits num=\""<<hits.size()<<"\">"<<endl;
  for (size_t i=0; i<hits.size(); ++i)
    os << hits[i];

  os << "</poselet_hits>"<<endl;
  os.close();
}

//#pragma optimize("",off)
void load_poselet_hits(poselet_hits_vector& hits,
                                         const char* filename) {
  ifstream is(filename);
  string line;
  getline(is,line,is.widen('\255'));
  is.close();
  using namespace rapidxml;
  xml_document<> doc;
  doc.parse<parse_non_destructive>(const_cast<char*>(line.c_str()));

  xml_node<> *root = doc.first_node("poselet_hits");
  assert(root);
  int n = boost::lexical_cast<int>(get_xml_value(root->first_attribute("num")));
  hits.resize(n);

  size_t i=0;
  xml_node<> *node = root->first_node("poselet_hit");
  assert(node);
  do {
    hits[i].init(node);
    ++i;
  } while ((node=node->next_sibling("poselet_hit")));
  assert(int(i)==n);
}
//#pragma optimize("",on)


// based on the locations of left and right shoulders and
// left and right hips, calculate bounds for torso
float_bounds keypoints2torso_bounds(const double_point torso[]) {
  double_point m_shoulder = (torso[0]+torso[1])/2;
  double_point m_hip      = (torso[2]+torso[3])/2;
  double_point ctr = (m_shoulder+m_hip)/2;
  double torso_len = l2_norm(m_hip-m_shoulder);
  double_point torso_dims(torso_len/config::get().TORSO_ASPECT_RATIO,
                          torso_len);
  return float_bounds(cast_point<float_point>(ctr-torso_dims/2),
                      cast_point<float_point>(torso_dims));
}
