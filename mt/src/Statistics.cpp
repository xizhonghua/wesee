/*
 * Statistics.cpp
 *
 *  Created on: Aug 26, 2013
 *      Author: zhonghua
 */

#include "Statistics.h"

Statistics::Statistics() {

	this->m_channels.resize(3);
	this->m_rblocks = 48;
	this->m_gblocks = 48;
	this->m_bblocks = 48;
	this->m_xblocks = 32;
	this->m_yblocks = 32;
	//this->m_ablocks = 3;
	this->m_postive_weight = 1;


	dim[0] = color_dim[0] = this->m_bblocks;
	dim[1] = color_dim[1] = this->m_gblocks;
	dim[2] = color_dim[2] = this->m_rblocks;
	dim[3] = this->m_xblocks;
	dim[4] = this->m_yblocks;

	//dim[5] = this->m_ablocks;
	this->m_data = cv::Mat(DIM, dim, CV_32SC1);
	this->m_data_count = cv::Mat(DIM, dim, CV_32SC1);
	this->m_data_color = cv::Mat(3, this->color_dim, CV_32SC1);

	this->m_xy_xblocks = 1200;
	this->m_xy_yblocks = 1200;

	this->m_data_xy = cv::Mat(this->m_xy_yblocks, this->m_xy_xblocks, CV_32SC1);

	this->m_stat_count = 0;
}

Statistics::~Statistics() {
// TODO Auto-generated destructor stub
}

void Statistics::save_data(const string& path){
	byte s = 0;

	FILE* fout;
	fout = fopen(path.c_str(), "wb");

	for(int ib=0;ib<this->m_bblocks;ib++)
		for(int ig=0;ig<this->m_gblocks;ig++)
			for(int ir=0;ir<this->m_rblocks;ir++)
				for(int ix=0;ix<this->m_xblocks;ix++)
					for(int iy=0;iy<this->m_yblocks;iy++)
						{
							Vec5i index = this->get_index(ib,ig,ir,ix,iy);
							s = SIGMOID((double)this->m_data.at<int>(index)/this->m_data_count.at<int>(index))*255;
							fwrite(&s, 1, 1, fout);
						}

//	Mat mat_xy = Mat(this->m_xy_yblocks, this->m_xy_xblocks, CV_8UC1);
//
//	for(int iy=0;iy<this->m_xy_yblocks;iy++)
//	for(int ix=0;ix<this->m_xy_xblocks;ix++)
//		{
//			mat_xy.at<byte>(iy, ix) = s = SIGMOID((double)this->m_data_xy.at<int>(iy, ix)/this->m_stat_count)*255;
//			fwrite(&s, 1, 1, fout);
//		}
//
//	for(int ib=0;ib<this->m_bblocks;ib++)
//		for(int ig=0;ig<this->m_gblocks;ig++)
//			for(int ir=0;ir<this->m_rblocks;ir++)
//			{
//				s = SIGMOID((double)this->m_data_color.at<int>(ib, ig, ir)/this->m_stat_count)*255;
//				fwrite(&s, 1, 1, fout);
//			}

	fclose(fout);

//	imwrite("xy.png", mat_xy);
}

bool Statistics::read_data(const string& path){
	FILE* fin = fopen(path.c_str(), "rb");
	if(!fin) return false;
	byte t = 0;
	for(int ib=0;ib<this->m_bblocks;ib++)
		for(int ig=0;ig<this->m_gblocks;ig++)
			for(int ir=0;ir<this->m_rblocks;ir++)
				for(int ix=0;ix<this->m_xblocks;ix++)
					for(int iy=0;iy<this->m_yblocks;iy++)
						{
							Vec5i index = this->get_index(ib,ig,ir,ix,iy);
							fread(&t, 1, 1, fin);
							this->m_data.at<int>(index) = t;
						}

//	for(int ix=0;ix<this->m_xy_xblocks;ix++)
//		for(int iy=0;iy<this->m_xy_yblocks;iy++)
//		{
//			fread(&t, 1, 1, fin);
//			this->m_data_xy.at<int>(iy, ix) = t;
//		}
//
//		for(int ib=0;ib<this->m_bblocks;ib++)
//			for(int ig=0;ig<this->m_gblocks;ig++)
//				for(int ir=0;ir<this->m_rblocks;ir++)
//				{
//					fread(&t, 1, 1, fin);
//					this->m_data_color.at<int>(ib, ig, ir) = t;
//				}

	fclose(fin);
	return true;
}

void Statistics::stat(const Mat& image, const Mat& profile){
	if(image.size != profile.size) {
		cerr<<"size does not match, can't stat"<<endl;
		return;
	}

	this->m_stat_count++;

	int width = image.cols;
	int height = image.rows;

	double aspect = (double)width/height;

	int a_b = this->get_aspect_block(aspect);

	byte G,B,R;

	Mat lab_image = image.clone();
	cv::cvtColor(lab_image, lab_image, CV_BGR2HSV);

	for(int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{
			byte label = profile.at<byte>(i,j);

			B = lab_image.at<Vec3b>(i,j)[0];
			G = lab_image.at<Vec3b>(i,j)[1];
			R = lab_image.at<Vec3b>(i,j)[2];

			int x_b = j*this->m_xblocks/width;
			int y_b = i*this->m_yblocks/height;
//			int xy_x_b = j*this->m_xy_xblocks/width;
//			int xy_y_b = i*this->m_xy_yblocks/height;
			int r_b = R*this->m_rblocks/256;
			int g_b = G*this->m_gblocks/256;
			int b_b = B*this->m_bblocks/256;

			Vec5i index = this->get_index(b_b,g_b,r_b,x_b,y_b);

			double value = 3*(label == 255 ? this->m_postive_weight : -1.0);

			this->m_data.at<int>(index) += value;
			this->m_data_count.at<int>(index)++;

//			this->m_data_xy.at<int>(xy_y_b, xy_x_b) += (label == 0 ? 2 : -1);
//			this->m_data_color.at<int>(Vec3i(b_b, g_b, r_b)) += value;
		}
}

void Statistics::predict(const Mat& image, Mat& trimap){
#define SQR(x) ((x)*(x))
#define DISTSQR(x1,y1,x2,y2) ((SQR((x1)-(x2))+SQR((y1)-(y2))))

	int width = image.cols;
	int height = image.rows;

	double aspect = (double)width/height;

	int a_b = this->get_aspect_block(aspect);

	byte G,B,R;

	int x_center = width/2;
	int y_center = height/2;
	double mean_dist = (SQR(x_center) + SQR(y_center));

	Mat lab_image = image.clone();
	cv::cvtColor(lab_image, lab_image, CV_BGR2HSV);

	for(int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{
			B = lab_image.at<Vec3b>(i,j)[0];
			G = lab_image.at<Vec3b>(i,j)[1];
			R = lab_image.at<Vec3b>(i,j)[2];

			int x_b = j*this->m_xblocks/width;
			int y_b = i*this->m_yblocks/height;
			int r_b = R*this->m_rblocks/256;
			int g_b = G*this->m_gblocks/256;
			int b_b = B*this->m_bblocks/256;
//			int xy_x_b = (double)j*this->m_xy_xblocks/width;
//			int xy_y_b = (double)i*this->m_xy_yblocks/height;


			Vec5i index = this->get_index(b_b,g_b,r_b,x_b,y_b);
			byte v0 = this->m_data.at<int>(index);
//			byte v1 = this->m_data_xy.at<int>(xy_y_b, xy_x_b);
//			byte v2 = this->m_data_color.at<int>(Vec3i(b_b, g_b, r_b));

			trimap.at<byte>(i,j) = v0; //std::max(0, std::min(255, (v0*1000 - v1*2)/1000));
		}
}


// ===========================================
Vec<int,5> Statistics::get_index(int b_b, int g_b, int r_b, int x_b, int y_b)
{
	Statistics::limit(b_b, 0, this->m_bblocks-1);
	Statistics::limit(g_b, 0, this->m_gblocks-1);
	Statistics::limit(r_b, 0, this->m_rblocks-1);
	Statistics::limit(x_b, 0, this->m_xblocks-1);
	Statistics::limit(y_b, 0, this->m_yblocks-1);

	return Vec<int,5>(b_b,g_b,r_b,x_b,y_b);
}
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
	if(aspect<0.8) return 0;
	if(aspect<1.2) return 1;
	return 2;
}
