#include "poselet_api.h"
#include "config.h"
#include "poselet_detector.h"
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
using namespace std;

template <typename ANY_VIEW> void api_image_to_view(
  const poselet_api::Image& img, ANY_VIEW& av);

////////////////////////////////////////////////////////////////////////////////
///
/// Define the scope of a run-time images and image views
///
////////////////////////////////////////////////////////////////////////////////

typedef boost::mpl::vector<
boost::gil::gray8_image_t, boost::gil::rgb8_image_t, boost::gil::bgr8_image_t,
boost::gil::rgb8_planar_image_t, boost::gil::rgba8_image_t> any_image_types;


typedef boost::gil::any_image<any_image_types>  any_image_t;
typedef any_image_t::view_t                     any_view_t;
typedef any_image_t::const_view_t               any_const_view_t;


namespace detail {
template <typename ANY_VIEW, typename T>
ANY_VIEW api_image_to_view_(const poselet_api::Image& img) {
  int_point imgSize((int)img._width,(int)img._height);
  const void* const* foo = img._planarPtrs;

  const T* const* channels=reinterpret_cast<const T* const*>(foo);
     //=reinterpret_cast<const T* const*>(img._planarPtrs);

  if (img._cs==poselet_api::Image::kGray) {
    return ANY_VIEW(boost::gil::interleaved_view(imgSize.x,imgSize.y,
       (const boost::gil::pixel<T,boost::gil::gray_layout_t>*)channels[0],
       img._rowBytes));
  }

  if (img._isPlanar) {
    // planar form
    switch (img._cs) {
      case poselet_api::Image::kRGB:
        return ANY_VIEW(boost::gil::planar_rgb_view (imgSize.x,imgSize.y,
           channels[0],channels[1],channels[2],img._rowBytes));
      case poselet_api::Image::kBGR:
        return ANY_VIEW(boost::gil::planar_rgb_view (imgSize.x,imgSize.y,
           channels[2],channels[1],channels[0],img._rowBytes));
      case poselet_api::Image::kCMYK:
        return ANY_VIEW(boost::gil::planar_cmyk_view(imgSize.x,imgSize.y,
           channels[0],channels[1],channels[2],channels[3],img._rowBytes));
      case poselet_api::Image::kRGBA:
        return ANY_VIEW(boost::gil::planar_rgba_view(imgSize.x,imgSize.y,
           channels[0],channels[1],channels[2],channels[3],img._rowBytes));
      case poselet_api::Image::kARGB:
        return ANY_VIEW(boost::gil::planar_rgba_view(imgSize.x,imgSize.y,
           channels[1],channels[2],channels[3],channels[0],img._rowBytes));
      default: assert(false);
    }
  } else {
    switch (img._cs) {
      case poselet_api::Image::kRGB:
        return ANY_VIEW(boost::gil::interleaved_view(imgSize.x,imgSize.y,
          (const boost::gil::pixel<T,boost::gil::rgb_layout_t >*)channels[0],
          img._rowBytes));
      case poselet_api::Image::kBGR:
        return ANY_VIEW(boost::gil::interleaved_view(imgSize.x,imgSize.y,
          (const boost::gil::pixel<T,boost::gil::bgr_layout_t >*)channels[0],
          img._rowBytes));
      case poselet_api::Image::kCMYK:
        return ANY_VIEW(boost::gil::interleaved_view(imgSize.x,imgSize.y,
          (const boost::gil::pixel<T,boost::gil::cmyk_layout_t>*)channels[0],
          img._rowBytes));
      case poselet_api::Image::kRGBA:
        return ANY_VIEW(boost::gil::interleaved_view(imgSize.x,imgSize.y,
        (const boost::gil::pixel<T,boost::gil::rgba_layout_t>*)channels[0],
          img._rowBytes));
      case poselet_api::Image::kARGB:
        // there is no native interleaved ARGB support.
        // It could be emulated with a planar image if needed
        break;
      default: assert(false);
    }
  }
  return ANY_VIEW();
}
}


template <typename ANY_VIEW>
void api_image_to_view(const poselet_api::Image& img, ANY_VIEW& av) {
  switch (img._depth) {
    case poselet_api::Image::k8Bit : av=::detail::api_image_to_view_<ANY_VIEW,
                                               boost::gil::bits8 > (img); break;
    case poselet_api::Image::k16Bit: av=::detail::api_image_to_view_<ANY_VIEW,
                                               boost::gil::bits16> (img); break;
    case poselet_api::Image::k32Bit: av=::detail::api_image_to_view_<ANY_VIEW,
                                               boost::gil::bits32f>(img); break;
    default: assert(false);
  }
}

template <typename L>
void api_image_to_view(const poselet_api::Image& img,
                       boost::gil::image_view<L>&) { throw "unimplemented"; }


class detection_mgr {
public:
  static detection_mgr& get() {
    static detection_mgr mgr;
    return mgr;
  }

  poselet_detector _detector;
private:
  detection_mgr() {};
};

namespace poselet_api {

extern "C" ErrorType InitDetector(const char* root_dir,
                                  const char* cat_name,
                                  bool sequential) {
  try {
    detection_mgr::get()._detector.init(root_dir, cat_name, sequential);
  } catch (...) { return ERROR_INVALID_ARGUMENTS; }
  return SUCCESS;
}

extern "C" bool POSELET_API IsInitialized() {
  return detection_mgr::get()._detector.model().num_poselets()>0;
}

extern "C" const char* POSELET_API ObjectType() {
  return detection_mgr::get()._detector.model().object_type();
}

extern "C" int POSELET_API NumAttributes() {
  return 0;
}


void output_poselet_hits(const poselet_hits_vector& ph,
                         PoseletHitCB poseletHitCB, int clusterID,
                         vector<vector<float> >* features=NULL) {
  using namespace boost::gil;
  PoseletHit phit;
  phit.clusterID=clusterID;
  phit.feature=NULL;
  phit.featureSize=0;
  for (size_t j=0; j<ph.size(); ++j) {
    phit.x0=iround(ph[j].bounds()._min.x);
    phit.y0=iround(ph[j].bounds()._min.y);
    phit.width=iround(ph[j].bounds().width());
    phit.height=iround(ph[j].bounds().height());
    phit.score = ph[j].score();
    phit.poseletID=ph[j].poselet_id();
    if (features) {
      phit.feature=&(*features)[j][0];
      phit.featureSize=(*features)[j].size();
    }
    poseletHitCB(phit);
  }
}

// keeps only the first maxHitsOfType of each poselet type
void filter_hits(poselet_hits_vector& hits, int maxPoseletID,
                 int maxHitsOfType) {
  assert(maxHitsOfType>0 && maxPoseletID>0);
  vector<bool> valid(hits.size(),true);
  vector<int> pcount_left(maxPoseletID,maxHitsOfType);
  size_t valid_count=hits.size();
  for (size_t i=0; i<hits.size(); ++i) {
    if (pcount_left[hits[i].poselet_id()]==0) {
      valid[i]=false;
      --valid_count;
    } else
      --pcount_left[hits[i].poselet_id()];
  }
  if (valid_count<hits.size()) {
    poselet_hits_vector new_hits;
    new_hits.reserve(valid_count);
    for (size_t i=0; i<hits.size(); ++i)
      if (valid[i])
        new_hits.push_back(hits[i]);
    swap(hits,new_hits);
  }
}

struct run_detector_fn {
  run_detector_fn(PoseletHitCB poseletHitCB, ObjectHitCB objectHitCB,
                  bool useBigQ, int maxFeatures, bool extractAttributes)
    : _poseletHitCB(poseletHitCB), _objectHitCB(objectHitCB), _useBigQ(useBigQ),
      _extractAttributes(extractAttributes), _maxFeatures(maxFeatures) {}

  typedef void result_type;

  template <typename View>
  void operator()(const View& v) {
    using namespace boost::gil;
    bgr32f255_image_t img32(v.dimensions());
    copy_and_convert_pixels(v, view(img32));

    if (!!_objectHitCB) {
      // detect objects
      assert(_maxFeatures==0);
      std::vector<object_hypothesis> obj_hits;
      poselet_hits_vector poselet_hits;
      detection_mgr::get()._detector.detect_objects(const_view(img32), _useBigQ,
         poselet_hits, obj_hits);
      for (size_t i=0; i<obj_hits.size(); ++i) {
        ObjectHit hit;
//  cerr << obj_hits[i].bounds()._min.x << " " <<  obj_hits[i].bounds()._min.y
//       << " "<< obj_hits[i].bounds().width() <<
//       << " "<< obj_hits[i].bounds().height()<<" "<<obj_hits[i].score()<<endl;
        hit.x0=iround(obj_hits[i].bounds()._min.x);
        hit.y0=iround(obj_hits[i].bounds()._min.y);
        hit.width=iround(obj_hits[i].bounds().width());
        hit.height=iround(obj_hits[i].bounds().height());
        hit.score = obj_hits[i].score();
        hit.clusterID=int(i);
        hit.category=0;
        hit.attributes=NULL;
        _objectHitCB(hit);
        if (!!_poseletHitCB)
          output_poselet_hits(obj_hits[i].poselet_hits(), _poseletHitCB,int(i));
      }
    } else {
      // detect just poselets
      assert(!!_poseletHitCB && !_extractAttributes);
      poselet_hits_vector poselet_hits;
      detection_mgr::get()._detector.detect_poselets(const_view(img32), poselet_hits);

      if (_maxFeatures>0) {
        // keep only the first _maxFeatures hits for each poselet type
        filter_hits(poselet_hits,
                    int(detection_mgr::get()._detector.model().num_poselets()),
                    _maxFeatures);
        std::vector<std::vector<float> > features;
        detection_mgr::get()._detector.get_features_of_hits(
				   const_view(img32),poselet_hits,features);
        output_poselet_hits(poselet_hits, _poseletHitCB, 0, &features);
      } else
        output_poselet_hits(poselet_hits, _poseletHitCB, 0);
    }
  }
private:
  PoseletHitCB _poseletHitCB;
  ObjectHitCB _objectHitCB;
  bool _useBigQ, _extractAttributes;
  int _maxFeatures;
};
extern "C" ErrorType POSELET_API RunDetector(
  const Image& img, PoseletHitCB poseletHitCB, ObjectHitCB objectHitCB,
  bool useBigQ,     // enable bigQ step if available
  int maxFeatures,  // when >0 number of top poselets to report + their features
  bool extractAttributes // whether to compute attributes
) {
  try {
    any_const_view_t av;
    api_image_to_view(img,av);
    run_detector_fn rdf(poseletHitCB,objectHitCB,
                        useBigQ,maxFeatures,extractAttributes);
    boost::gil::apply_operation(av, rdf);
  } catch (...) { return ERROR_INVALID_ARGUMENTS; }
  return SUCCESS;
}


}
