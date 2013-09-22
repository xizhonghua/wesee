#pragma once

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <numeric>

namespace boost { namespace gil {

namespace detail {
// Returns the type of a pixel value that correspond to SrcPixelRef
// but with a channel ChannelOp::result_type
// This is a shortcut class used in channel_deref_fn
template <typename SrcPixelRef, typename ChannelOp>
class vt {
  typedef typename color_space_type<
    typename remove_reference<SrcPixelRef>::type>::type cst;
public:
  typedef typename pixel_value_type<
  typename ChannelOp::result_type, layout<cst> >::type type;
};
}

// Models PixelDereferenceAdaptor (non-mutable) that returns a pixel after
// transforming its channels with ChannelOp. ChannelOp must model unary function
// that takes and returns a model of ChannelValueConcept
template <typename SrcPixelRef, typename ChannelOp>
struct channel_deref_fn : public std::unary_function<SrcPixelRef,
                            typename detail::vt<SrcPixelRef,ChannelOp>::type> {
  typedef channel_deref_fn const_t;
  typedef typename detail::vt<SrcPixelRef,ChannelOp>::type      value_type;
  typedef value_type reference;
  typedef value_type const_reference;
  BOOST_STATIC_CONSTANT(bool, is_mutable = false);

  // Takes a (const reference) to a pixel
  // and returns a copy with its channels transformed
  value_type operator()(SrcPixelRef x) const {
    value_type ret;
    gil::static_transform(x,ret,ChannelOp());
    return ret;
  }
};

// Helper class to construct the type and create an instance of a view
// that upon pixel access transforms the channels of the pixel with ChannelOp
template <typename SrcView, typename ChannelOp>
struct channel_transform_view_type {
private:
  GIL_CLASS_REQUIRE(SrcView, boost::gil, ImageViewConcept)
  typedef channel_deref_fn<typename SrcView::const_t::reference,
                           ChannelOp> deref_t;
  typedef typename SrcView::template add_deref<deref_t> add_ref_t;
public:
  typedef typename add_ref_t::type type;
  static type make(const SrcView& src, ChannelOp op) {
    return add_ref_t::make(src,deref_t());
  }
};

/// \ingroup ImageViewTransformationsNthChannel
template <typename View, typename ChannelOp>
typename channel_transform_view_type<View, ChannelOp>::type
channel_transform_view(const View& src, ChannelOp op) {
  return channel_transform_view_type<View, ChannelOp>::make(src,op);
}

///////////////////////////////////////
//////////////////////////////////////

namespace detail {
struct max_fn {
  template <typename T1, typename T2>
  typename channel_traits<T1>::value_type
  operator()(const T1& x, const T2& y) const {
    return (x>y) ? x : channel_traits<T1>::value_type(y);
  }
};

struct channel_min_value_fn {
  template <typename T>
  void operator()(T& val) const { val=channel_traits<T>::min_value(); }
};

// Accumulates the maximum pixel value
template <typename CPixelRef, typename PixelValue>
struct max_pixel : public std::unary_function<CPixelRef,void> {
  PixelValue _max;

  max_pixel() {
    // initialize the pixel with the minimum value for the channel
    static_for_each(_max,detail::channel_min_value_fn());
  }
  void operator()(CPixelRef x) {
    static_transform(_max,x,_max,detail::max_fn());
  }
};
}

// Returns a pixel value of the same type as that of view.
// Each channel is set to the maximum of the channel in the view
template <typename View, typename PixelValue>
void view_max_values(const View& v, PixelValue& init) {
  init=for_each_pixel(v, detail::max_pixel<typename View::const_t::reference,
                                           PixelValue>())._max;
}



///////////////////////////////////////
//////////////////////////////////////

// Debugging function used to compare nearly identical views. Returns
// the x,y location and difference magnitude of the pixel that differs the most
template <typename View>
std::pair<gil::point2<int>,double>
max_absdiff_pixels(const View& v1, const View& v2) {
  assert(v1.dimensions()==v2.dimensions());
  gil::point2<int> max_coords(0,0);
  double max_diff=0;
  for (int y=0; y<v1.height(); ++y) {
    typename View::x_iterator v1xit=v1.row_begin(y);
    typename View::x_iterator v2xit=v2.row_begin(y);
    for (int x=0; x<v1.width(); ++x) {
      double diff=0;
      for (int c=0; c<gil::num_channels<View>::value; ++c)
        diff+=fabs(v1xit[x][c]-v2xit[x][c]);
      if (diff>max_diff) {
        max_diff=diff;
        max_coords=gil::point2<int>(x,y);
      }
    }
  }
  return std::make_pair(max_coords,max_diff);
}

///////////////////////////////////////
//////////////////////////////////////

struct sample_map {
  int src,dst;
  double w;

  sample_map(int s, int d, double ww) : src(s), dst(d), w(ww) {}
};

static void get_src2dst_map(int src_n, int dst_n,
                            std::vector<sample_map>& mapping) {
  double scale = dst_n/double(src_n);
  double scaleInv = 1/scale;
  mapping.reserve(src_n+2*dst_n);
  double ya0f=0, ya1f=scaleInv;
  for (int yb=0; yb<dst_n; ++yb) {
    int ya0=int(ceil(ya0f));
    int ya1=int(ya1f);
    if (ya0-ya0f > 1e-3)
      mapping.push_back(sample_map(ya0-1,yb, (ya0-ya0f)*scale));
    for (int ya=ya0; ya<ya1; ++ya)
      mapping.push_back(sample_map(ya,yb,scale));
    if (ya1f-ya1 > 1e-3)
      mapping.push_back(sample_map(ya1,yb,(ya1f-ya1)*scale));
    ya0f=ya1f;
    ya1f+=scaleInv;
  }
}

template <typename SrcView, typename DstView>
void downsample_width(const SrcView& src, const DstView& dst) {
  assert(src.width()<=dst.width() && src.height()==dst.height());
  typedef typename SrcView::reference src_pixel_t;
  typedef typename DstView::reference dst_pixel_t;

  std::vector<sample_map> src2dst_map;
  get_src2dst_map(src.width(), dst.width(), src2dst_map);
  std::vector<sample_map>::const_iterator end_it=src2dst_map.end();

  int h=src.height();
  for (int y=0; y<h; ++y) {
    typename SrcView::x_iterator src_it=src.row_begin(y);
    typename DstView::x_iterator dst_it=dst.row_begin(y);
    for (std::vector<sample_map>::const_iterator it=src2dst_map.begin();
         it!=end_it; ++it)
      detail::add_dst_mul_src<src_pixel_t,double,dst_pixel_t>()
        (src_it[it->src], it->w, dst_it[it->dst]);
  }
}

template <typename SrcView, typename DstView>
void downsample_view(const SrcView& src, const DstView& dst) {
  assert(src.width()>=dst.width() && src.height()>=dst.height());

  image<typename SrcView::value_type> tmpImage(src.height(), dst.width());
  downsample_width(src,transposed_view(view(tmpImage))); // resize horizontally
  downsample_width(const_view(tmpImage), transposed_view(dst)); // vertically
}

} } // namespace

// Compares two ranges. Returns an iterator to the element in the first range
// that differs the most from the corresponding one in the second range, and
// returns the difference. The difference is L1 norm
// V models Number, *It is convertible to V, It is a forward iterator
template <typename It>
std::pair<It,typename std::iterator_traits<It>::value_type>
max_diff(It first, It last, It first2) {
  It max_it=first;
  typename std::iterator_traits<It>::value_type max_diff=0;
  while (first!=last) {
    typename std::iterator_traits<It>::value_type diff=abs(*first-*first2);
    if (diff>max_diff) {
      max_diff=diff;
      max_it=first;
    }
    ++first;
    ++first2;
  }
  return make_pair(max_it,max_diff);
}
