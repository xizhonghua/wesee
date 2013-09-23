// Support for 4-channel RGB where the last channel is ignored

#ifndef GIL_RGBX
#define GIL_RGBX

#include <cstddef>
#include <boost/gil/gil_config.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/gil/rgb.hpp>
#include <boost/gil/planar_pixel_iterator.hpp>

namespace boost { namespace gil {

struct unused_t {};
typedef mpl::vector4<red_t, green_t, blue_t, unused_t> rgbx_t;

typedef layout<rgbx_t> rgbx_layout_t;
typedef layout<rgbx_t, mpl::vector4_c<int,2,1,0,3> > bgrx_layout_t;
typedef layout<rgbx_t, mpl::vector4_c<int,1,2,3,0> > xrgb_layout_t;
typedef layout<rgbx_t, mpl::vector4_c<int,3,2,1,0> > xbgr_layout_t;

template <typename IC>
inline
typename type_from_x_iterator<planar_pixel_iterator<IC,rgbx_t> >::view_t
planar_rgbx_view(std::size_t width, std::size_t height,
                 IC r, IC g, IC b, IC x,
                 std::ptrdiff_t rowsize_in_bytes) {
    typedef typename
      type_from_x_iterator<planar_pixel_iterator<IC,rgbx_t> >::view_t RView;
    return RView(width, height,
      typename RView::locator(planar_pixel_iterator<IC,rgbx_t>(r,g,b,x),
                              rowsize_in_bytes));
}

GIL_DEFINE_BASE_TYPEDEFS(8,  bgrx)
GIL_DEFINE_BASE_TYPEDEFS(8s, bgrx)
GIL_DEFINE_BASE_TYPEDEFS(16, bgrx)
GIL_DEFINE_BASE_TYPEDEFS(16s,bgrx)
GIL_DEFINE_BASE_TYPEDEFS(32 ,bgrx)
GIL_DEFINE_BASE_TYPEDEFS(32s,bgrx)
GIL_DEFINE_BASE_TYPEDEFS(32f,bgrx)

GIL_DEFINE_BASE_TYPEDEFS(8,  xrgb)
GIL_DEFINE_BASE_TYPEDEFS(8s, xrgb)
GIL_DEFINE_BASE_TYPEDEFS(16, xrgb)
GIL_DEFINE_BASE_TYPEDEFS(16s,xrgb)
GIL_DEFINE_BASE_TYPEDEFS(32 ,xrgb)
GIL_DEFINE_BASE_TYPEDEFS(32s,xrgb)
GIL_DEFINE_BASE_TYPEDEFS(32f,xrgb)

GIL_DEFINE_BASE_TYPEDEFS(8,  xbgr)
GIL_DEFINE_BASE_TYPEDEFS(8s, xbgr)
GIL_DEFINE_BASE_TYPEDEFS(16, xbgr)
GIL_DEFINE_BASE_TYPEDEFS(16s,xbgr)
GIL_DEFINE_BASE_TYPEDEFS(32 ,xbgr)
GIL_DEFINE_BASE_TYPEDEFS(32s,xbgr)
GIL_DEFINE_BASE_TYPEDEFS(32f,xbgr)

GIL_DEFINE_ALL_TYPEDEFS(8,  rgbx)
GIL_DEFINE_ALL_TYPEDEFS(8s, rgbx)
GIL_DEFINE_ALL_TYPEDEFS(16, rgbx)
GIL_DEFINE_ALL_TYPEDEFS(16s,rgbx)
GIL_DEFINE_ALL_TYPEDEFS(32 ,rgbx)
GIL_DEFINE_ALL_TYPEDEFS(32s,rgbx)
GIL_DEFINE_ALL_TYPEDEFS(32f,rgbx)

// Color conversion from rgbx to any pixel. Supports homogeneous pixels only
template <typename C2>
struct default_color_converter_impl<rgbx_t,C2> {
  template <typename P1, typename P2>
  void operator()(const P1& src, P2& dst) const {
    typedef typename channel_type<P1>::type T1;
    default_color_converter_impl<rgb_t,C2>()(
      pixel<T1,rgb_layout_t>(get_color(src,red_t()),
                             get_color(src,green_t()),
                             get_color(src,blue_t()))
      ,dst);
  }
};

} }  // namespace boost::gil

#endif
