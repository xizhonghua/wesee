// Support for floating point channel with range 0..255

#ifndef GIL_CHANNEL255
#define GIL_CHANNEL255

#include <boost/gil/gil_config.hpp>

namespace boost { namespace gil {


// Floating point channel with range 0..255

struct float255  { static float apply() { return 255.0f; } };
typedef scoped_channel_value<float,float_zero,float255> bits32f255;
GIL_DEFINE_BASE_TYPEDEFS(32f255, gray);
GIL_DEFINE_BASE_TYPEDEFS(32f255, bgr);
GIL_DEFINE_ALL_TYPEDEFS(32f255,  rgb);


} }  // namespace boost::gil

#endif
