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

	bool enarge = width > org.cols;

	Mat out;
	cv::resize(org, out, Size(width, height), 0, 0, (enarge ? cv::INTER_CUBIC : cv::INTER_AREA));

	return out;
}

Mat MatHelper::resize(const Mat& ori, int width, int height){
	bool enarge = width > ori.cols;

	Mat out;
	cv::resize(ori, out, Size(width, height), 0, 0, (enarge ? cv::INTER_CUBIC : cv::INTER_AREA));

	return out;
}

Mat MatHelper::read_image(const string& filename, int long_edge){
	Mat input = imread(filename, cv::IMREAD_UNCHANGED);
	if(long_edge == 0) return input;
	return MatHelper::resize(input, long_edge);
}


Mat MatHelper::read_image_ch(const string& string, int ch){
	Mat input = imread(string.c_str(), cv::IMREAD_UNCHANGED);
	vector<Mat> chs;
	cv::split(input, chs);
	return chs[ch];
}
