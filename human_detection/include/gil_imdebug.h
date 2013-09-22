#pragma once

#include <boost/gil/gil_all.hpp>
#ifdef USE_IMDEBUG
#include <imdebug.h>
#else
void imdebug(const char*,...) {}
#endif
#include <sstream>
#include <string>


template <typename View>
struct imdebugger_name {
};

template <>
struct imdebugger_name<boost::gil::rgb8_view_t> {
  static const char* value;
};

template <>
struct imdebugger_name<boost::gil::bgr8_view_t> {
  static const char* value;
};

template <>
struct imdebugger_name<boost::gil::gray32f_view_t> {
  static const char* value;
};

template <typename View>
void debug_display_view(const View& v) {
  std::stringstream str;
  str << imdebugger_name<View>::value
      << " w=" << v.width() << " h=" << v.height() << " %p" << std::ends;
  imdebug(str.str().c_str(),interleaved_view_get_raw_data(v));
}
