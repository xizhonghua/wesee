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


// ==============================================
// static method
// ==============================================
void Matting::dump_training_results(){
	//TODO dump training results
	cout<<"TODO dump training results"<<endl;
}
