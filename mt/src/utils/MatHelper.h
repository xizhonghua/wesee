/*
 * MatHelper.h
 *
 *  Created on: Aug 24, 2013
 *      Author: zhonghua
 */

#ifndef MATHELPER_H_
#define MATHELPER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class MatHelper {
public:
	static Mat resize(const Mat& ori, int long_edge);
};

#endif /* MATHELPER_H_ */
