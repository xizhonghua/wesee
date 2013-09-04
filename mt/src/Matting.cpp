/*
 * Matting.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */

#include <iostream>
using namespace std;

#include "Matting.h"
#include "setting.h"
#include "MatHelper.h"
#include "Timer.h"

extern setting g_setting;

Matting::Matting() {
	// TODO Auto-generated constructor stub

}

Matting::~Matting() {
	// TODO Auto-generated destructor stub
}

int Matting::train(const Mat& image, const Mat& profile){
	return 0;
}

double Matting::grabCut(const Mat& ori, const Mat& min, const Mat& trimap, const Rect& boundRect, const vector<Point>& contour, Mat& output){
	Timer t;
	t.restart();

	Mat input, mask;
	mask.create(min.size(), CV_8UC1);
	mask = cv::GC_BGD;

	Mat bgdModel;
	Mat fgdModel;

	int width = min.cols;
	int height = min.rows;

	mask(boundRect) = cv::GC_PR_FGD;

	for(int j=boundRect.x;j<boundRect.x + boundRect.width;j++)
		for(int i=boundRect.y;i<boundRect.y + boundRect.height;i++)
		{
			int value = trimap.at<byte>(i,j);

			double dist = cv::pointPolygonTest(contour, Point2i(j,i), true);

			if(dist > 0){
				// inside the contour
				if(value >= 240){
					mask.at<byte>(i,j) = cv::GC_FGD;
				}
				else if(value <= 12)
				{
					mask.at<byte>(i,j) = cv::GC_PR_BGD;
				}
			}
			if(dist < 0)
			{
				if(value <= 12)
					mask.at<byte>(i,j) = cv::GC_BGD;
				else if(value <= 30)
					mask.at<byte>(i,j) = cv::GC_PR_BGD;
			}
		}

	cv::cvtColor(min, min, CV_BGR2XYZ);

	cv::grabCut(min, mask, boundRect, bgdModel, fgdModel, g_setting.max_refine_iterations, cv::GC_INIT_WITH_MASK);

	Mat seg = Mat(min.size(), CV_8UC1);
	seg = 255;

	seg.copyTo(output, mask & 1);

	double cost = t.getElapsedMilliseconds();

	output = MatHelper::resize(output, ori.cols,  ori.rows);

	return cost;
}

int Matting::mat(const Statistics& stat, const Mat& ori, Mat& output, Mat& predict_raw, Mat& predit_drawing) {
	// TODO implement matting algorithm

	Mat input = MatHelper::resize(ori, LONG_EDGE_PX);
	predict_raw = Mat::zeros(ori.rows, ori.cols, CV_8UC1);

	stat.predict(ori, predict_raw);

	Mat predict_s = MatHelper::resize(predict_raw, LONG_EDGE_PX);

	//cv::fastNlMeansDenoising(predict_s, predict_s, 11);
	//blur( predict_s, predict_s, Size(11,11) );
	medianBlur(predict_s, predict_s, 5);

	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	int thresh = 70;
	int max_thresh = 255;
	RNG rng(12345);

	/// Detect edges using Threshold
	threshold( predict_s, threshold_output, thresh, 255, THRESH_BINARY );
	/// Find contours
	findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	/// Approximate contours to polygons + get bounding rects and circles
	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<float>radius( contours.size() );

	Rect bestBoundRect;
	vector<Point> betContourPoly;
	int best_index;
	for( int i = 0; i < contours.size(); i++ )
	{
		approxPolyDP( Mat(contours[i]), contours_poly[i], 1, true );
		boundRect[i] = boundingRect( Mat(contours_poly[i]) );
		if(boundRect[i].area() > bestBoundRect.area())  {
			bestBoundRect = boundRect[i];
			betContourPoly = contours_poly[i];
			best_index = i;
		}
	}

	bestBoundRect.x -= bestBoundRect.width * 0.15;
	bestBoundRect.x = std::max(bestBoundRect.x, 0);
	bestBoundRect.width *= 1.3;
	bestBoundRect.width = std::min(predict_s.cols - bestBoundRect.x, bestBoundRect.width);

	bestBoundRect.y -= bestBoundRect.height * 0.15;
	bestBoundRect.y = std::max(bestBoundRect.y, 0);
	bestBoundRect.height *= 1.3;
	bestBoundRect.height = std::min(predict_s.rows - bestBoundRect.y, bestBoundRect.height);

	if(g_setting.output_prediction)
	{
		// draw bounding box & contour
		Scalar WHITE = Scalar( 255, 255, 255 );
		Scalar BLACK = Scalar( 0, 0, 0 );

		predit_drawing = predict_s.clone();

		rectangle( predit_drawing, bestBoundRect.tl(), bestBoundRect.br(), WHITE, 2, 8, 0 );
		drawContours( predit_drawing, contours_poly, best_index, WHITE, 1, 8, vector<Vec4i>(), 0, Point() );
	}

	Mat result;
	double cost = grabCut(ori, input, predict_s, bestBoundRect, contours_poly[best_index], result);

	output = MatHelper::resize(result, ori.cols, ori.rows);
	medianBlur(output, output, 7);

	cv::threshold(output, output, 128, 255, CV_THRESH_BINARY);

	output = 255 - output;

	return 1;
}

double Matting::evaluate(const Mat& ground_truth, const Mat& result){
	const Mat& g = ground_truth;
	const Mat& r = result;

	if(g.rows != r.rows || g.cols != r.cols){
		cerr<<"! Warning ! size doesn't match"
			<<" groud_truth = "<<g.rows<<"x"<<g.cols
			<<" result = "<<r.rows<<"x"<<r.cols<<endl;
	}

	int min_rows = std::min(g.rows, r.rows);
	int min_cols = std::min(g.cols, r.cols);

	int inter_count = 0;
	int union_count = 0;

	for(int i=0; i<min_rows; i++)
		for(int j=0; j<min_cols; j++)
		    {
		    	unsigned char r_g = g.at<unsigned char>(i,j);
		    	unsigned char r_r = r.at<unsigned char>(i,j);
		    	if(r_g == 255 && r_r == 0) inter_count++;
		    	if(r_g == 255 || r_r == 0) union_count++;
		    }

	if(union_count == 0) return 0.0;

	double p = (double)inter_count/union_count;

	return p;
}


// ==============================================
// static method
// ==============================================
void Matting::dump_training_results(){
	//TODO dump training results
	cout<<"TODO dump training results"<<endl;
}
