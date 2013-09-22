#pragma once

#include <boost/gil/utilities.hpp>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <limits>

inline int intround(float x ) {
  return static_cast<int>(x + (x < 0.0f ? -0.5f : 0.5f)); }
inline int intround(double x) {
  return static_cast<int>(x + (x < 0.0  ? -0.5  : 0.5 )); }

inline boost::gil::point2<int> intround(const boost::gil::point2<float >& p)  {
  return boost::gil::point2<int>(intround(p.x),intround(p.y)); }
inline boost::gil::point2<int> intround(const boost::gil::point2<double>& p)  {
  return boost::gil::point2<int>(intround(p.x),intround(p.y)); }

////////////////////////////////////////////////////////////////////////////////
/// CLASS bounds
///
/// A 2D bounding box represented as a pair of top left (_min) and bottom right
/// (_max) points. The coordinates of _max must be greater or equal to those in
/// _min.
///
////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct bounds {
  typedef boost::gil::point2<T> point_type;

  bounds() : _min(std::numeric_limits<T>::max(), std::numeric_limits<T>::max()),
    _max(std::numeric_limits<T>::min(), std::numeric_limits<T>::min()) {}
  bounds(const point_type& minPt, const point_type& maxPt,bool)
    : _min(minPt), _max(maxPt)                   {}
  bounds(const T& xMin, const T& yMin, const T& xSize, const T& ySize)
    : _min(xMin,yMin), _max(xMin+xSize,yMin+ySize) {}
  bounds(const T& xMin, const T& yMin, const T& xMax, const T& yMax, bool)
    : _min(xMin,yMin), _max(xMax,yMax) {}
  bounds(const point_type& minPt, const point_type& sz)
    : _min(minPt), _max(minPt+sz) {}
  template <typename T1> explicit bounds(const bounds<T1>& p)
    : _min(p._min), _max(p._max) {}

  bounds(const bounds& b)                     : _min(b._min), _max(b._max) {}
  const bounds& operator=(const bounds& b) {
    _min=b._min; _max=b._max; return *this;
  }

  bool            intersects(const bounds& b)    const;
  template <class T1> bool contains(const boost::gil::point2<T1>& pt)    const {
    return _min.x<=pt.x && pt.x<=_max.x && _min.y<=pt.y && pt.y<=_max.y;
  }
  bounds& operator+=(const point_type& pt) { _min+=pt; _max+=pt; return *this; }
  bounds& operator-=(const point_type& pt) { _min-=pt; _max-=pt; return *this; }

  bool contains(const bounds& b) const;
  bool is_null() const    { return width()<=0 || height()<=0; }

  void set_to_intersection(const bounds& b);
  void set_to_union(const bounds& b);
  void set_to_union(const point_type& p);

  boost::gil::point2<double>  center()                        const;

  const point_type size()       const { return _max-_min; }     // obsolete
  const point_type dimensions() const { return _max-_min; }
  T               width()       const { return T(_max.x-_min.x); }
  T               height()      const { return T(_max.y-_min.y); }

  const T& operator[](std::size_t i) const {
    return reinterpret_cast<const T*>(this)[i];
  }// unsafe
  T& operator[](std::size_t i) { return reinterpret_cast<T*>(this)[i]; }// unsaf

  point_type _min,_max;
};

template <typename T> std::ostream& operator<<(std::ostream& os,
                                               const bounds<T>& b) {
  return os << b._min<<" "<<b._max<<" ";
}
template <typename T> std::istream& operator>>(std::istream& is, bounds<T>& b) {
  return is >> b._min >> b._max;
}

template <typename T> inline
bool operator==(const bounds<T>& b1, const bounds<T>& b2) {
  return (b1._min==b2._min && b1._max==b2._max); }

template <typename T> inline
bool operator!=(const bounds<T>& b1, const bounds<T>& b2) { return  !(b1==b2); }

template <typename T> inline
bounds<T> operator+(const bounds<T>& b, const boost::gil::point2<T>& pt) {
  return bounds<T>(b._min+pt,b.size()); }

template <typename T> inline
bounds<T> operator-(const bounds<T>& b, const boost::gil::point2<T>& pt) {
  return bounds<T>(b._min-pt,b.size()); }

template <typename T>
bool bounds<T>::intersects(const bounds& b) const {
  return !(_min.x>b._max.x || _min.y>b._max.y ||
           _max.x<b._min.x || _max.y<b._min.y);
}

template <typename T>
bool bounds<T>::contains(const bounds& b) const {
  return contains(b._min) && contains(b._max);
}

template <typename T>
boost::gil::point2<double> bounds<T>::center() const {
  return boost::gil::point2<double>((_max.x+_min.x)/2.0f,(_max.y+_min.y)/2.0f);
}

template <typename T> inline
boost::gil::point2<T> get_dimensions(const bounds<T>& b) {
  return b._max - b._min;
}

// Can result in incorrect bounds if the two bounds don't intersect
template <typename T>
void bounds<T>::set_to_intersection(const bounds& b) {
  _min.x=std::max(_min.x,b._min.x);
  _min.y=std::max(_min.y,b._min.y);
  _max.x=std::min(_max.x,b._max.x);
  _max.y=std::min(_max.y,b._max.y);
}

template <typename T>
inline bounds<T> intersect(const bounds<T>& b1, const bounds<T>& b2) {
  return bounds<T>(std::max(b1._min.x, b2._min.x),
                   std::max(b1._min.y, b2._min.y),
                   std::min(b1._max.x, b2._max.x),
                   std::min(b1._max.y, b2._max.y),true);
}

// unfortunately union is a C++ keyword
template <typename T>
inline bounds<T> union_bounds(const bounds<T>& b1,const bounds<T>& b2) {
  return bounds<T>(std::min(b1._min.x,b2._min.x),
                   std::min(b1._min.y,b2._min.y),
                   std::max(b1._max.x,b2._max.x),
                   std::max(b1._max.y,b2._max.y),true);
}

template <typename T>
void bounds<T>::set_to_union(const bounds& b) {
  _min.x=std::min(_min.x,b._min.x);
  _min.y=std::min(_min.y,b._min.y);
  _max.x=std::max(_max.x,b._max.x);
  _max.y=std::max(_max.y,b._max.y);
}

template <typename T>
void bounds<T>::set_to_union(const boost::gil::point2<T>& p) {
  _min.x=std::min(_min.x,p.x);
  _min.y=std::min(_min.y,p.y);
  _max.x=std::max(_max.x,p.x);
  _max.y=std::max(_max.y,p.y);
}

template <typename T> inline
bounds<double> operator*(const bounds<T>& b, double scale) {
  return bounds<double>(b._min*scale,b._max*scale,true);
}

template <typename T> inline T area(const bounds<T>& b) {
  return b.width()*b.height(); }


inline const bounds<int> iround(const bounds<float >& p)     {
  return bounds<int>(intround(p._min),intround(p._max),true);
}
inline const bounds<int> iround(const bounds<double>& p)     {
  return bounds<int>(intround(p._min),intround(p._max),true);
}

template <typename Bounds>
double bounds_overlap(const Bounds& b1, const Bounds& b2) {
  double x0=std::max(b1._min.x, b2._min.x);
  double x1=std::min(b1._max.x, b2._max.x);
  if (x1<=x0) return 0;

  double y0=std::max(b1._min.y, b2._min.y);
  double y1=std::min(b1._max.y, b2._max.y);
  if (y1<=y0) return 0;

  double int_area = (x1-x0) * (y1-y0);
  return int_area / double(area(b1) + area(b2) - int_area);
}

template <typename Bounds1, typename Bounds2, typename Scalar>
inline void interpolate_bounds(const Bounds2& b2, Scalar w2, Bounds1& accum,
                                 Scalar w1=1) {
  interpolate_point(b2._min,w2,accum._min,w1);
  interpolate_point(b2._max,w2,accum._max,w1);
}

template <typename Bounds, typename Scalar>
Bounds get_interpolated_bounds(const Bounds& b1, Scalar w1,
                               const Bounds& b2, Scalar w2) {
  return Bounds(get_interpolated_point(b1._min,w1,b2._min,w2),
                get_interpolated_point(b1._max,w1,b2._max,w2), true);
}

template <typename MetaView>
inline bounds<int> view_bounds(const MetaView& v) {
  return bounds<int>(0,0,(int)v.width(),(int)v.height()); }

typedef bounds<int> int_bounds;
typedef bounds<float> float_bounds;
