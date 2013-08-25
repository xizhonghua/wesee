/*
 * Matting.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */

#include <iostream>
using namespace std;

#include "Matting.h"

Matting::Matting() {
	// TODO Auto-generated constructor stub

}

Matting::~Matting() {
	// TODO Auto-generated destructor stub
}

int Matting::train(const Mat& image, const Mat& profile){
	return 0;
}

int Matting::mat(const Mat& image, Mat& out) {
	// TODO implement matting algorithm

	image.copyTo(out);

	//Example: convert to BW
	for(int i=0; i<out.rows; i++)
	    for(int j=0; j<out.cols; j++)
	    {
	    	unsigned char c =
	    			out.at<cv::Vec3b>(i,j)[0] * 0.3 +
	    			out.at<cv::Vec3b>(i,j)[1] * 0.6 +
	    			out.at<cv::Vec3b>(i,j)[2] * 0.1;
	    	out.at<cv::Vec3b>(i,j)[0] = out.at<cv::Vec3b>(i,j)[1] = out.at<cv::Vec3b>(i,j)[2];
	    }

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
		    	if(r_g == 255 && r_r == 255) inter_count++;
		    	if(r_g == 255 || r_r == 255) union_count++;
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
