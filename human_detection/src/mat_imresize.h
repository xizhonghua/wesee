#pragma once

#include <boost/gil/gil_all.hpp>
#include <vector>

void resample(double *A, double *B, int ha, int hb, int w, int nCh);

template<typename SrcView, typename DstView>
  void mat_resize(const SrcView& src, const DstView& dst) {
  using namespace boost::gil;
  size_t sh = src.height(), sw = src.width(), sc = src.num_channels();
  size_t dh = dst.height(), dw = dst.width(), dc = dst.num_channels();
  assert(sc==dc);

  std::vector<double> A(sh*sw*sc), T(dh*sw*sc,0), B(dh*dw*dc,0);
  // src comes from C++ which uses row-major indexing, resizing algorithm works
  // with Matlab which uses column-major indexing,
  // therefore conversion is needed to make A column-major.
  int k=0;
  for (size_t ch=0; ch < sc; ++ch)
    for (size_t x=0; x<sw; ++x)
      for (size_t y=0; y<sh; ++y)
        A[k++]=(double)src(x,y)[ch];

  resample(&A[0], &T[0], sh, dh, sw, sc);
  resample(&T[0], &B[0], sw, dw, dh, sc);

  k = 0;
  for (size_t ch=0; ch < sc; ++ch)
    for (size_t x=0; x<dw; ++x)
      for (size_t y=0; y<dh; ++y) {
        dst(x,y)[ch] = (typename channel_type<DstView>::type)(B[k++]);
      }
}
