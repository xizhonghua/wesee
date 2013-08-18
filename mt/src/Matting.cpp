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
		cout<<"! Warning ! size doesn't match"
			<<" groud_truth = "<<g.rows<<"x"<<g.cols
			<<" result = "<<r.rows<<"x"<<r.cols;
		return 0.0;
	}

	int inter_count = 0;
	int union_count = 0;

	for(int i=0; i<g.rows; i++)
		for(int j=0; j<g.cols; j++)
		    {
		    	unsigned char r_g = g.at<int>(i,j);
		    	unsigned char r_r = r.at<cv::Vec3b>(i,j)[0]; // only use red channel now

		    	// TODO
		    	if(r_g == 0 && r_r > 128) inter_count++;
		    	if(r_g == 0 || r_r > 120) union_count++;
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
