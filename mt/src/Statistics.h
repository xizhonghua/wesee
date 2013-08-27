/*
 * Statistics.h
 *
 *  Created on: Aug 26, 2013
 *      Author: zhonghua
 */

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits.h>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

#include "def.h"

#define DIM (5)
typedef Vec<int,5> Vec5i;

struct SField{
	string name;
	double min;
	double max;
	double sum;
	int count;

	SField(){
		min = INT_MAX;
		max = INT_MIN;
		sum = 0;
		count = sum;
	}

	void add_sample(double value){
		if(value > max) max = value;
		if(value < min) min = value;
		sum += value;
		++count;
	}

	double get_average(){
		return (count == 0 ? 0 : sum/count);
	}
};

class Statistics {
public:
	Statistics();
	virtual ~Statistics();
	void stat(const Mat& image, const Mat& profile);
	void predict(const Mat& image, Mat& trimap);
	void save_data(const string& path);
	bool read_data(const string& path);
private:
	int get_aspect_block(double aspect);
	Vec6i get_index(int b_b, int g_b, int r_b, int x_b, int y_b, int a_b);
	Vec5i get_index(int b_b, int g_b, int r_b, int x_b, int y_b);
	static void limit(int& value, int min, int max) { if(value > max) value = max; if(value<min) value = min;}

	int m_xblocks;
	int m_yblocks;
	int m_ablocks;
	int m_rblocks;
	int m_gblocks;
	int m_bblocks;
	int m_xy_xblocks;
	int m_xy_yblocks;
	int m_stat_count;
	int dim[6];
	int color_dim[3];
	double m_postive_weight;
	Mat m_data;
	Mat m_data_xy;
	Mat m_data_color;
	SField m_aspect;
	vector<SField> m_channels;
};

#endif /* STATISTICS_H_ */
