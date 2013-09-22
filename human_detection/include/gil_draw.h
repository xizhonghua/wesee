#pragma once

#include <boost/gil/gil_all.hpp>
#include "bounds.h"

typedef boost::gil::point2<int> int_point;

template <typename View>
void draw_bounds(const View& v, const typename View::value_type& color,
                 const int_bounds& bounds, int thickness) {
  int th1=thickness/2;
  fill_bounds(v,color,int_bounds(bounds._min.x-th1,bounds._min.y-th1,
                                 bounds.width()+thickness,thickness));
  fill_bounds(v,color,int_bounds(bounds._min.x-th1,bounds._max.y-th1,
                                 bounds.width()+thickness,thickness));
  fill_bounds(v,color,int_bounds(bounds._min.x-th1,bounds._min.y,
                                 thickness,bounds.height()));
  fill_bounds(v,color,int_bounds(bounds._max.x-th1,bounds._min.y,
                                 thickness,bounds.height()));
}

template <typename View>
void fill_bounds(const View& v, const typename View::value_type& color,
                 const int_bounds& bounds) {
  int_bounds b=intersect(bounds,int_bounds(0,0,int(v.width()),int(v.height())));
  for (int y=b._min.y; y<b._max.y; ++y) {
    typename View::x_iterator xit=v.row_begin(y);
    for (int x=b._min.x; x<b._max.x; ++x)
      xit[x]=color;
  }
}

namespace boost { namespace gil { namespace detail {

template <typename Weight>
struct blend_channel {
  Weight _w;
  explicit blend_channel(Weight w) : _w(w) {}

  template <typename SrcChannel, typename DstChannel>
  void operator()(const SrcChannel& src, DstChannel& dst) const {
    dst = DstChannel(dst*(1-_w)) + DstChannel(src*_w);
  }
};

// dst = dst*(1-w) + DST_TYPE(src * w)
template <typename SrcP,typename Weight,typename DstP>
struct blend_pixel {
  void operator()(const SrcP& src, Weight weight, DstP& dst) const {
    static_for_each(src,dst, blend_channel<Weight>(weight));
  }
};
} } } // namespace boost::gil::detail

// v = v*(1-alpha*mask) + color*(alpha*mask)
template <typename View, typename GrayView>
void blend_mask(const View& v, const typename View::value_type& color,
                float alpha, const GrayView& mask) {
  assert(v.dimensions()==mask.dimensions());
  typedef typename View::value_type Pixel;

  for (int y=0; y<v.height(); ++y) {
    typename View::x_iterator vit=v.row_begin(y);
    typename GrayView::x_iterator mit=mask.row_begin(y);
    for (int x=0; x<v.width(); ++x)
      boost::gil::detail::blend_pixel<Pixel,float,Pixel>()
        (color,mit[x][0]*alpha,vit[x]);
  }
}

struct string2bitmap {
  static const string2bitmap& get() {
    static string2bitmap gS2b;
    return gS2b;
  }
  void get_bitmap(const std::string& number,
                  boost::gil::gray32f_image_t& bitmap, float scale=1) const;
private:
  string2bitmap() {}
  static const int_point _char_dims; // = [6,8]
  static const float _mask[11][6*8];
};

template <typename View>
void render_text(const View& v, const int_point& top_left,
                 const std::string& txt, const typename View::value_type& color,
                 float alpha=1, float scale=1) {
  boost::gil::gray32f_image_t bitmap;
  string2bitmap::get().get_bitmap(txt,bitmap,scale);
  int_bounds isect = intersect(view_bounds(v),
                               view_bounds(const_view(bitmap))+top_left);
  if (!isect.is_null())
    blend_mask(subimage_view(v, isect._min.x, isect._min.y,
                             isect.width(), isect.height()),
               color, alpha, subimage_view(const_view(bitmap),
       isect._min.x-top_left.x, isect._min.y-top_left.y,
       isect.width(), isect.height()));
}


