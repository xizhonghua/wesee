/*
 * Matting.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */

#include "Matting.h"

Matting::Matting() {
	// TODO Auto-generated constructor stub

}

Matting::~Matting() {
	// TODO Auto-generated destructor stub
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
