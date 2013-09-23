#pragma once
#include <vector>
#include <map>

template <typename ElementInIt, typename ElementOutIt,// iterator over elements
  typename DistType,
  typename DistFn,  // DistType distance(const Element& x, const Element& y)
  typename MergeFn> // Element merge(const Element& x, const Element& y)
void agglomerative_cluster(ElementInIt elementsIt, size_t num_elements,
                           const DistFn& dist_fn, const MergeFn& merge_fn,
                           DistType threshold, ElementOutIt result) {
  using namespace std;

  if (num_elements == 0) return;
  typedef typename iterator_traits<ElementInIt>::value_type element_t;
  vector<element_t> clusters(elementsIt, elementsIt+num_elements);
  vector<bool> valid(num_elements,true);
  clusters.reserve(num_elements*2);
  valid.reserve(num_elements*2);

  typedef pair<size_t,size_t> el_pair_t;
  typedef map<DistType,el_pair_t> distances_map_t;
  distances_map_t distances;
  for (size_t i=0; i<num_elements-1; ++i)
    for (size_t j=i+1; j<num_elements; ++j)
      distances.insert(make_pair(dist_fn(clusters[i],clusters[j]),
                                 make_pair(i,j)));

  while (!distances.empty()) {
    // get the closest pair
    typename distances_map_t::iterator it=distances.begin();
    if (it->first > threshold)
      break;

    el_pair_t closest_pair=it->second;
    distances.erase(it);
    if (!valid[closest_pair.first] || !valid[closest_pair.second])
      continue; // This pair had already merged elements. Go to the next pair

    // add the merged element
    clusters.push_back(merge_fn(clusters[closest_pair.first],
                                clusters[closest_pair.second]));
    valid.push_back(true);
    valid[closest_pair.first]=false;
    valid[closest_pair.second]=false;
    for (size_t i=0; i<clusters.size()-1; ++i)
      if (valid[i])
        distances.insert(make_pair(dist_fn(clusters.back(),clusters[i]),
                                   make_pair(i,clusters.size()-1)));
  }

  for (size_t i=0; i<clusters.size(); ++i)
    if (valid[i])
      (*result)++ = clusters[i];
}
