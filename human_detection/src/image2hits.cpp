#include "image2hits.h"
#include "poselet_hit.h"
#include <boost/timer.hpp>
#include <list>

using namespace std;
using namespace boost::gil;

void image2hits::hog2poselet_hits(const hog_features& hog,
                                  poselet_hits_vector& hits) const {
  boost::timer t;
  const float pix_per_bin=float(hog_config::get().pix_per_bin());

  static int count=0;
  static double totaltime=0;
  for (size_t dims_type=0; dims_type<_model->dims_groups().size(); ++dims_type){
    int_point template_dims=_model->dims_groups()[dims_type]._dims;
    const std::vector<size_t>& pids
      =_model->dims_groups()[dims_type]._poselet_ids;

    std::vector<float> feature(hog.bin_size()*template_dims.x*template_dims.y);
    std::vector<double> scores(pids.size());

    int_point p;
    for (p.y=0; p.y<=hog.num_bins().y-template_dims.y; ++p.y)
      for (p.x=0; p.x<=hog.num_bins().x-template_dims.x; ++p.x) {
        // extract the feature at the current bin coords
        hog.get_feature(p,template_dims,feature);

        for (int pd=0; pd<int(pids.size()); ++pd) {
          scores[pd] = (*_model)[pids[pd]].inner_product(&feature[0]);
          ++count;
        }
        for (size_t pd=0; pd<pids.size(); ++pd)
          if (scores[pd]>=0)
            hits.push_back(
              poselet_hit(int(pids[pd]), scores[pd],
                          float_bounds(p.x*pix_per_bin,
                                       p.y*pix_per_bin,
                                       (template_dims.x+1)*pix_per_bin,
                                       (template_dims.y+1)*pix_per_bin)));
      }
  }
  totaltime+=t.elapsed();
//  cerr << "Total num evals:" << count << " detected poselets:"
//       << hits.size() << " in " << totaltime << " sec." << endl;
}

//#pragma optimize("",off)
void image2hits::nonmax_suppress_hits(const poselets_model& model,
                                            const poselet_hits_vector& hits_in,
                                            poselet_hits_vector& hits_out)
{
  if (hits_in.empty()) return;
  list<poselet_hit> merged;

  // Split the hits by poselet id
  vector<vector<poselet_hit> > hits_by_poselet(model.num_poselets());
  for (size_t i=0; i<hits_in.size(); ++i)
    hits_by_poselet[hits_in[i].poselet_id()].push_back(hits_in[i]);

  for (size_t p=0; p<model.num_poselets(); ++p) {
    // sort the hits of poselet p by score
    std::sort(hits_by_poselet[p].begin(), hits_by_poselet[p].end());

    // append them at the end of merged. Set first_s as iterator
    // to the beginning of the sorted elements of p in merged
    list<poselet_hit>::iterator first_s;
    if (merged.empty()) {
      merged.assign(hits_by_poselet[p].begin(), hits_by_poselet[p].end());
      first_s=merged.begin();
    } else {
      first_s=merged.end(); --first_s;
      merged.insert(merged.end(),
                    hits_by_poselet[p].begin(),
                    hits_by_poselet[p].end());
      ++first_s;
    }

    // remove any of the hits that are overlapped by more than 0.5 by another
    // hit of the same poselet with higher score
    list<poselet_hit>::iterator listend=merged.end();
    if (hits_by_poselet[p].size()>1) {
      list<poselet_hit>::iterator fit=first_s;
      do {
        assert(fit!=listend);
        list<poselet_hit>::iterator bit=fit;
        ++bit;
        if (bit==listend)
          break;
        do {
          if (bounds_overlap(fit->bounds(), bit->bounds())>=0.5)
            bit=merged.erase(bit);
          else
            ++bit;
        } while (bit!=listend);
        ++fit;
      } while (fit!=listend);
    }

    // set the score of the survived hits to probability. It is still sorted
    // because of monotonicity of the logistic
    for (list<poselet_hit>::iterator it=first_s; it!=listend; ++it)
      it->set_score(model[p].probability(float(it->score())));

    // merge the survivor hits into the list. The list is sorted by score
    std::inplace_merge(merged.begin(), first_s, merged.end());
  }

  hits_out.assign(merged.begin(), merged.end());
}

