#include "gil_draw.h"
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>

using namespace boost::gil;
using namespace std;

void string2bitmap::get_bitmap(const string& number, gray32f_image_t& bitmap,
                               float scale) const {
  gray32f_image_t norm_bitmap(number.size()*_char_dims.x,_char_dims.y);
  for (size_t c=0; c<number.size(); ++c) {
    int digit=(number[c]=='.' ? 10 : number[c]-'0');
    if (digit<0 || digit>10)
      digit=10;
    copy_pixels(
      interleaved_view(_char_dims.x,_char_dims.y,
                       gray32f_ptr_t(&_mask[digit]),_char_dims.x*sizeof(float)),
      subimage_view(view(norm_bitmap),
                    int(c)*_char_dims.x,0,_char_dims.x,_char_dims.y));
  }
  if (scale!=1) {
    bitmap.recreate(iround(norm_bitmap.width()*scale),
                    iround(norm_bitmap.height()*scale));
    resize_view<bilinear_sampler>(const_view(norm_bitmap),view(bitmap));
  } else
    swap(bitmap,norm_bitmap);
}

const int_point string2bitmap::_char_dims(6,8);

const float string2bitmap::_mask[11][6*8] = {
        {0,0,0,0,0,0,
         0,0,1,1,1,0,
         0,1,0,0,0,1,
         0,1,0,0,1,1,
         0,1,0,1,0,1,
         0,1,1,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,0},

        {0,0,0,0,0,0,
         0,0,0,1,0,0,
         0,0,1,1,0,0,
         0,0,0,1,0,0,
         0,0,0,1,0,0,
         0,0,0,1,0,0,
         0,0,0,1,0,0,
         0,0,1,1,1,0},

        {0,0,0,0,0,0,
         0,0,1,1,1,0,
         0,1,0,0,0,1,
         0,0,0,0,0,1,
         0,0,0,1,1,0,
         0,0,1,0,0,0,
         0,1,0,0,0,0,
         0,1,1,1,1,1},

        {0,0,0,0,0,0,
         0,1,1,1,1,1,
         0,0,0,0,0,1,
         0,0,0,0,1,0,
         0,0,0,1,1,0,
         0,0,0,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,0},

        {0,0,0,0,0,0,
         0,0,0,0,1,0,
         0,0,0,1,1,0,
         0,0,1,0,1,0,
         0,1,0,0,1,0,
         0,1,1,1,1,1,
         0,0,0,0,1,0,
         0,0,0,0,1,0},

        {0,0,0,0,0,0,
         0,1,1,1,1,1,
         0,1,0,0,0,0,
         0,1,1,1,1,0,
         0,0,0,0,0,1,
         0,0,0,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,0},

        {0,0,0,0,0,0,
         0,0,0,1,1,1,
         0,0,1,0,0,0,
         0,1,0,0,0,0,
         0,1,1,1,1,0,
         0,1,0,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,0},

        {0,0,0,0,0,0,
         0,1,1,1,1,1,
         0,0,0,0,0,1,
         0,0,0,0,1,0,
         0,0,0,1,0,0,
         0,0,1,0,0,0,
         0,0,1,0,0,0,
         0,0,1,0,0,0},

        {0,0,0,0,0,0,
         0,0,1,1,1,0,
         0,1,0,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,0,
         0,1,0,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,0},

        {0,0,0,0,0,0,
         0,0,1,1,1,0,
         0,1,0,0,0,1,
         0,1,0,0,0,1,
         0,0,1,1,1,1,
         0,0,0,0,0,1,
         0,0,0,0,1,0,
         0,1,1,1,0,0},

        {0,0,0,0,0,0,
         0,0,0,0,0,0,
         0,0,0,0,0,0,
         0,0,0,0,0,0,
         0,0,0,0,0,0,
         0,0,0,0,0,0,
         0,0,0,0,0,0,
         0,0,1,0,0,0},
};
