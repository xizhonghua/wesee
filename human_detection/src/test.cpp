#include <vector>
#include <numeric>
#include "util.h"
  
void foo() {
  std::vector<double> vec;
  int bar=std::accumulate(vec.begin(),vec.end(),0);
}


 
