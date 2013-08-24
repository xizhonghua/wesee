/*
 * MatHelper.cpp
 *
 *  Created on: Aug 24, 2013
 *      Author: zhonghua
 */

#include "MatHelper.h"

Mat MatHelper::resize(const Mat& org, int long_edge){

	double aspect = (double)org.cols/org.rows;

	int width, height;

	if(aspect >= 1){
		width = long_edge;
		height = width / aspect;
	}else {
		height = long_edge;
		width = height*aspect;
	}

	Mat out;
	cv::resize(org, out, Size(width, height), 0, 0, cv::INTER_AREA);

	return out;
}

