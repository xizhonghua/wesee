/*
 * MatHelper.h
 *
 *      Author: zhonghua
 */

#ifndef MATHELPER_H_
#define MATHELPER_H_

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class MatHelper {
public:
	static Mat resize(const Mat& ori, int long_edge);
	static Mat resize(const Mat& ori, int width, int height);

	static Mat read_image(const string& string, int long_edge=0);

	static Mat read_image_ch(const string& string, int ch);
};

#endif /* MATHELPER_H_ */
