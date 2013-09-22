#include "hog_features.h"
#include <math.h>

using namespace std;

extern int_point get_topleft_hog_grid(const int_point& img_dims,
                                      int_point& num_bins);
int_point get_topleft_hog_grid(const int_point& img_dims,
                               int_point& num_bins) {
  int_point bandwidth(hog_config::get().CELL_DIMS.x /
                      hog_config::get().N_HOG_DIMS.x,
                      hog_config::get().CELL_DIMS.y /
                      hog_config::get().N_HOG_DIMS.y);
  int_point NUM_HOG_BINS = hog_config::get().N_HOG_DIMS;
  int_point HOG_CELL_DIMS = hog_config::get().CELL_DIMS;
  num_bins = int_point(int(floor((img_dims.x-2)/
                       float(bandwidth.x)) - NUM_HOG_BINS.x+1),
                       int(floor((img_dims.y-2)/
                            float(bandwidth.y)) - NUM_HOG_BINS.y+1));

  return int_point(int(floorf (0.5f*(img_dims.x
                    - (num_bins.x-1)*bandwidth.x - HOG_CELL_DIMS.x))),
                   int(floorf (0.5f*(img_dims.y
                    - (num_bins.y-1)*bandwidth.y - HOG_CELL_DIMS.y))));
}

int hog_features_generator::footprint(const int_point& img_dim) const
{
  int_point num_bins;
  int_point topleft = get_topleft_hog_grid(img_dim, num_bins);
  const int hog_bin_size = NUM_ANGLES * NUM_HOG_BINS.x * NUM_HOG_BINS.y;
  const int num_hog_bins = num_bins.x * num_bins.y;
  return num_hog_bins*hog_bin_size*sizeof(float);
}

hog_features_generator::hog_features_generator()
 : HOG_WTSCALE(2), HOG_NORM_EPS(1.0f), HOG_NORM_EPS2(0.01f),
   HOG_NORM_MAXVAL(0.2f),
   NUM_HOG_BINS(hog_config::get().N_HOG_DIMS),
   bandwidth(hog_config::get().CELL_DIMS.x/hog_config::get().N_HOG_DIMS.x,
             hog_config::get().CELL_DIMS.y/hog_config::get().N_HOG_DIMS.y),
   HOG_CELL_DIMS(hog_config::get().CELL_DIMS),
   NUM_ANGLES(hog_config::get().NUM_ANGLES) {
  float_point var2;
  for (int i = 0; i < 2; i++){
    var2[i] = HOG_CELL_DIMS[i] / (2*HOG_WTSCALE);
    var2[i] = var2[i]*var2[i]*2;
  }
  float_point half_bin(NUM_HOG_BINS.x/2.0f, NUM_HOG_BINS.y/2.0f);
  float_point cenBand(HOG_CELL_DIMS.x/2.0f, HOG_CELL_DIMS.y/2.0f);

  _w.resize(boost::extents[HOG_CELL_DIMS.x][HOG_CELL_DIMS.y]);
  _pt.resize(boost::extents[HOG_CELL_DIMS.x][HOG_CELL_DIMS.y]);
  for (int x = 0; x < HOG_CELL_DIMS.x; x++){
    for (int y = 0; y < HOG_CELL_DIMS.y; y++)
      {
        float xx = (x - 0.5f*HOG_CELL_DIMS.x);
        float yy = (y - 0.5f*HOG_CELL_DIMS.y);
        _w[x][y] = expf( -(xx*xx / var2.x) -(yy*yy / var2.y));

        _pt[x][y][0] = half_bin[0] - 0.5f +
          (x+0.5f-cenBand[0]) / bandwidth[0];
        _pt[x][y][1] = half_bin[1] - 0.5f +
          (y+0.5f-cenBand[1]) / bandwidth[1];
      }
  }
}


unsigned char sqrt(unsigned char x) {
#define DECL(x,n,foo) (unsigned char) sqrt(double(foo+n)),
#define DECL16(x,n,foo) BOOST_PP_REPEAT(16,DECL,n*16)
  static const unsigned char lookup[256] = {
    BOOST_PP_REPEAT(16,DECL16,0)
  };
#undef DECL
#undef DECL16

  return lookup[x];
}


void hog_features::get_feature(
    const int_point& top_left, const int_point& template_dims,
    std::vector<float>& feature) const
{
  int_point p=top_left;
  float* f=&feature[0];
  int width=template_dims.x*bin_size();
  assert(int(feature.size())==width*template_dims.y);
  for (int y=0; y<template_dims.y; ++y) {
    const float* src=get_hog_at_bin(p);
    std::copy(src,src+width,f);
    f+=width;
    ++p.y;
  }
}

