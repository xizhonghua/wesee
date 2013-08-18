/*
 * Matting.h
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */

#ifndef MATTING_H_
#define MATTING_H_

#include <opencv2/core/core.hpp>
using namespace cv;

class Matting {
public:
	Matting();
	virtual ~Matting();
	int mat(const Mat& image, Mat& output);
};

#endif /* MATTING_H_ */
