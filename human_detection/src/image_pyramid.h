#pragma once

#include "config.h"
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include "util.h"
#include "mat_imresize.h"
#include <iostream>

template <typename Op>
void pyramid_op(const int_point& img_dims,
                int_point target_dims, Op& op) {
  using namespace boost::gil;
   //std::cout<<"pyramid_op"<<std::endl;
  if (!target_dims.x && !target_dims.y) {
    target_dims = img_dims;
  }

  double scale=std::max(1.0,
    sqrt(config::get().DETECTION_IMG_MIN_NUM_PIX
            / double(target_dims.x * target_dims.y)));
  if (scale==1)
    scale=std::min(1.0,
      sqrt(config::get().DETECTION_IMG_MAX_NUM_PIX
              / double(target_dims.x * target_dims.y)));

  double img2padded_scale =
        std::min(target_dims.x / double(img_dims.x),
                 target_dims.y / double(img_dims.y));

  int_point MARGIN = config::get().IMAGE_MARGIN;

  double min_scale=std::max(MARGIN.x/double(target_dims.x),
                            MARGIN.y/double(target_dims.y));
  //std::cout<<"min_scale = "<<min_scale<<std::endl;
  while (true) {
    int_point padded_img_size(int(target_dims.x * scale),
                              int(target_dims.y * scale));

    if (std::min(padded_img_size.x,padded_img_size.y)<64)
      break;

    int_point img_size(int(img_dims.x * scale * img2padded_scale),
                       int(img_dims.y * scale * img2padded_scale));
    int_point diff = padded_img_size - img_size;
    assert(diff.x>=0 && diff.y>=0 && diff.x*diff.y==0);

    op(img_size, MARGIN+diff, scale*img2padded_scale);

    scale/=config::get().PYRAMID_SCALE_RATIO;
    if (scale<min_scale)
      break;
  }
}

template <typename View, typename Op>
class pyramid_generator : boost::noncopyable {
  typedef boost::gil::image<typename View::value_type,false> img_t;
public:
  pyramid_generator(const View& v, Op& op) : _v(v), _op(op) {}

  void apply(const int_point& target_dims) {
    pyramid_op(cast_point<int_point>(_v.dimensions()), target_dims, *this);
  }

  void operator()(const int_point& init_size,
                  const int_point& MARGIN, double scale) {
    using namespace boost::gil;
    typedef point2<ptrdiff_t> ptrdiff_point;
    int_point half_margin=MARGIN/2;

    ptrdiff_point marginp = ptrdiff_pt(MARGIN);
    ptrdiff_point half_marginp=ptrdiff_pt(half_margin);

    ptrdiff_point init_sizep=ptrdiff_pt(init_size);
    _img.recreate(init_sizep+marginp);
    typename img_t::const_view_t cv = const_view(_img);
    typename img_t::view_t mv = view(_img);

    mat_resize(_v, subimage_view(mv,half_marginp,init_sizep));

    copy_pixels(
      subimage_view(cv, half_margin.x,half_margin.y,init_size.x,half_margin.y),
      flipped_up_down_view(subimage_view(mv,
           half_margin.x,0,init_size.x,half_margin.y)));

    copy_pixels(
      subimage_view(cv, half_margin.x,init_size.y,init_size.x,half_margin.y),
      flipped_up_down_view(subimage_view(mv,
           half_margin.x,init_size.y+half_margin.y,init_size.x,half_margin.y)));

    copy_pixels(
      subimage_view(cv, half_margin.x,0,half_margin.x,init_size.y+MARGIN.y),
      flipped_left_right_view(subimage_view(mv,
           0,0,half_margin.x,init_size.y+MARGIN.y)));

    copy_pixels(
      subimage_view(cv,init_size.x, 0,half_margin.x,init_size.y+MARGIN.y),
      flipped_left_right_view(subimage_view(mv,
           init_size.x+half_margin.x,0,half_margin.x,init_size.y+MARGIN.y)));

    _op(cv, -half_margin + int_point(1,1), scale);
  }
private:
  img_t _img;
  const View& _v;
  Op& _op;
};

// Given an image, generates scaled versions of it and calls
// the provided function object with them.
// The scaled version are padded using reflection.
// In addition the padding might be larger horizontally or vertically to
// satisfy the given aspect ratio
template <typename View, typename Op>
void generate_image_pyramid(const View& v,
      const int_point& target_dims, Op& op) {
  pyramid_generator<View,Op> gen(v, op);
  gen.apply(target_dims);
}

