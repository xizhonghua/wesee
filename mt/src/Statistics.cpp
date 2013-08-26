/*
 * Statistics.cpp
 *
 *  Created on: Aug 26, 2013
 *      Author: zhonghua
 */

#include "Statistics.h"

Statistics::Statistics() {
	const int DIM = 6;

	this->m_channels.resize(3);
	this->m_rblocks = 30;
	this->m_gblocks = 50;
	this->m_bblocks = 10;
	this->m_xblocks = 40;
	this->m_yblocks = 40;
	this->m_ablocks = 7;
	this->m_postive_weight = 0.2;

	int dim[DIM];
	dim[0] = this->m_bblocks;
	dim[1] = this->m_gblocks;
	dim[2] = this->m_rblocks;		//BGR
	dim[3] = this->m_xblocks;
	dim[4] = this->m_yblocks;
	dim[5] = this->m_ablocks;
	this->m_data = cv::Mat(DIM, &dim[0], CV_16SC1);
}

Statistics::~Statistics() {
// TODO Auto-generated destructor stub
}

void Statistics::save_data(const string& path){


}

void Statistics::read_data(const string& path){

}

void Statistics::stat(const Mat& image, const Mat& profile){
	if(image.size != profile.size) {
		cerr<<"size does not match, can't stat"<<endl;
		return;
	}

	int width = image.cols;
	int height = image.rows;

	double aspect = (double)width/height;

	this->m_aspect.add_sample(aspect);

	int a_b = this->get_aspect_block(aspect);

	byte G,B,R;

	Vec6i index;
	for(int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{
			byte label = profile.at<Vec4b>(i,j)[3];

			B = image.at<Vec3b>(i,j)[0];
			G = image.at<Vec3b>(i,j)[1];
			R = image.at<Vec3b>(i,j)[2];
			this->m_channels[0].add_sample(G);
			this->m_channels[1].add_sample(B);
			this->m_channels[2].add_sample(R);

			int x_b = j*this->m_xblocks/width;
			int y_b = i*this->m_yblocks/height;
			int r_b = R*this->m_rblocks/256;
			int g_b = G*this->m_gblocks/256;
			int b_b = B*this->m_bblocks/256;

			Vec6i index = this->get_index(b_b,g_b,r_b,x_b,y_b,a_b);

			int value = 10*(label == 255 ? this->m_postive_weight : -1);

			this->m_data.at<int>(index) += value;
		}
}

void Statistics::predict(const Mat& image, Mat& trimap){
	int width = image.cols;
	int height = image.rows;

	double aspect = (double)width/height;

	this->m_aspect.add_sample(aspect);

	int a_b = this->get_aspect_block(aspect);

	byte G,B,R;

	for(int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{
			B = image.at<Vec3b>(i,j)[0];
			G = image.at<Vec3b>(i,j)[1];
			R = image.at<Vec3b>(i,j)[2];
			this->m_channels[0].add_sample(G);
			this->m_channels[1].add_sample(B);
			this->m_channels[2].add_sample(R);

			int x_b = j*this->m_xblocks/width;
			int y_b = i*this->m_yblocks/height;
			int r_b = R*this->m_rblocks/256;
			int g_b = G*this->m_gblocks/256;
			int b_b = B*this->m_bblocks/256;

			//Vec6i index = this->get_index(b_b,g_b,r_b,x_b,y_b,a_b);

			double value = 0; //this->m_data.at<int>(index);

			static double da_weight[3] = {80, 10, 10};
			static int da_offset[3] = {-1, 0, 1};

			for(int da =0; da<3; da++)
			{
				Vec6i index = this->get_index(b_b,g_b,r_b,x_b,y_b,a_b+ da_offset[da]);
				value += this->m_data.at<int>(index)*da_weight[da];
			}

			double sig = SIGMOID(value/100);

			trimap.at<byte>(i,j) = sig*255;
		}
}


// ===========================================
Vec6i Statistics::get_index(int b_b, int g_b, int r_b, int x_b, int y_b, int a_b)
{
	Statistics::limit(b_b, 0, this->m_bblocks-1);
	Statistics::limit(g_b, 0, this->m_gblocks-1);
	Statistics::limit(r_b, 0, this->m_rblocks-1);
	Statistics::limit(x_b, 0, this->m_xblocks-1);
	Statistics::limit(y_b, 0, this->m_yblocks-1);
	Statistics::limit(a_b, 0, this->m_ablocks-1);

	return Vec6i(b_b,g_b,r_b,x_b,y_b,a_b);
}

int Statistics::get_aspect_block(double aspect){
	if(aspect<0.5) return 0;
	if(aspect<0.65) return 1;	// classic portrait
	if(aspect<0.8) return 2;
	if(aspect<1.2) return 3;
	if(aspect<1.5) return 4;	// classic landscape;
	if(aspect<2.0) return 5;
	return 6;
}
