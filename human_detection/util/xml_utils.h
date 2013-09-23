#pragma once

#include "rapidxml.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>

template <typename T>
std::string get_xml_name(const T* node) {
  return std::string(node->name(), node->name()+node->name_size());
}

template <typename T>
std::string get_xml_value(const T* node) {
  return std::string(node->value(), node->value()+node->value_size());
}

template <typename T>
void read_xml_vector(const std::string& st1, size_t sz, std::vector<T>& vec) {
  std::string st=st1;
  size_t found=st.find("NaN");
  while (found!=std::string::npos) {
    st.replace(found,3," 0 ");
    found=st.find("NaN",found);
  }

  std::stringstream str(st);

  vec.resize(sz);
  T p;
  for (size_t i=0; i<sz; ++i) {
    str>>p;
    assert(!str.eof() && !str.bad());
    vec[i]=p;
  }
  assert(!str.eof());
  int foo;
  str>>foo;
  assert(str.eof());
}
