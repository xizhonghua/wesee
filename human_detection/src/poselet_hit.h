#pragma once

#include "config.h"
#include "bounds.h"
#include "rapidxml.hpp"
#include <boost/format.hpp>
#include <boost/gil/gil_all.hpp>
class hypothesis;
class hit {
public:
  hit() : _score(-1),_bounds() {}
  hit(double score, const float_bounds& b) : _score(score), _bounds(b) {}

  hit(const hit& ph) : _score(ph._score), _bounds(ph._bounds) {}
  hit& operator=(const hit& ph) {
    hit tmp(ph);
    swap(*this,tmp);
    return *this;
  }

  friend bool operator==(const hit& a, const hit& b) {
    return a._bounds==b._bounds && a._score==b._score;
  }

  void init(rapidxml::xml_node<>* node);
  void transform(const float_point& tr, float sc) {
    _bounds._min=(_bounds._min+tr)*sc;
    _bounds._max=(_bounds._max+tr)*sc;
  }
  friend void swap(hit& a, hit& b) {
    using std::swap;
    swap(a._score,b._score);
    swap(a._bounds,b._bounds);
  }

  void set_score(double score) { _score=score; }
  double score() const { return _score; }
  const float_point& min_pt() const { return _bounds._min; }
  float_point dims() const { return _bounds.dimensions(); }
  const float_bounds& bounds() const { return _bounds; }

  friend bool operator<(const hit& a, const hit& b) {
    return a.score() > b.score();
  }

  template <typename View, typename Color>
    void draw(const View& v, const Color& c, int thickness=1) const {
    typename View::value_type color;
    boost::gil::color_convert(c,color);
    draw_bounds(v,color,cast_bounds<int_bounds>(_bounds),thickness);

    render_text(v, cast_point<int_point>(_bounds._min) +
                int_point(thickness,thickness),
                str(boost::format("%4.2f") % _score), c, 1, 1);
  }
private:
  double _score;
  float_bounds _bounds;
};

class poselet_hit : public hit {
public:
  poselet_hit() {}
  poselet_hit(int pid, double score, const float_bounds& bounds)
    : hit(score,bounds), _poselet_id(pid) {}

  poselet_hit(const poselet_hit& ph) : hit(ph), _poselet_id(ph._poselet_id) {}
  poselet_hit& operator=(const poselet_hit& ph) {
    poselet_hit tmp(ph);
    swap(*this,tmp);
    return *this;
  }

  void init(rapidxml::xml_node<>* node);

  float_bounds get_torso_bounds(hypothesis& hyps) const;

  friend void swap(poselet_hit& a, poselet_hit& b) {
    using std::swap;
    swap(static_cast<hit&>(a),static_cast<hit&>(b));
    swap(a._poselet_id,b._poselet_id);
  }

  friend bool operator==(const poselet_hit& a, const poselet_hit& b) {
    return static_cast<const hit&>(a)==static_cast<const hit&>(b) &&
           a._poselet_id==b._poselet_id; }

  int poselet_id() const { return _poselet_id; }

private:
  int _poselet_id;
};


typedef std::vector<hit>                 hits_vector;
typedef std::vector<poselet_hit> poselet_hits_vector;

std::ostream& operator<<(std::ostream& os, const hit& hit);
std::ostream& operator<<(std::ostream& os, const poselet_hit& hit);

void save_hits(const poselet_hits_vector& hits,
               const std::string& filename);

void  save_poselet_hits(const poselet_hits_vector& h, const char* f);
void  load_poselet_hits(      poselet_hits_vector& h, const char* f);

float_bounds keypoints2torso_bounds(const double_point torso[]);
