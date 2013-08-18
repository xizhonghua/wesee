/*
 * Image.h
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class Image {
public:
	Image();
	virtual ~Image();
};

#endif /* IMAGE_H_ */
