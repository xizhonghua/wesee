/*
 * Statistics.cpp
 *
 *  Created on: Aug 26, 2013
 *      Author: zhonghua
 */

#include "Statistics.h"

Statistics::Statistics() {

	this->m_channels.resize(3);
	this->m_rblocks = 20;
	this->m_gblocks = 20;
	this->m_bblocks = 20;
	this->m_xblocks = 50;
	this->m_yblocks = 50;
	this->m_ablocks = 3;
	this->m_postive_weight = 0.5;


	dim[0] = this->m_bblocks;
	dim[1] = this->m_gblocks;
	dim[2] = this->m_rblocks;
	dim[3] = this->m_xblocks;
	dim[4] = this->m_yblocks;
	dim[5] = this->m_ablocks;
	this->m_data = cv::Mat(DIM, dim, CV_16SC1);
}

Statistics::~Statistics() {
// TODO Auto-generated destructor stub
}

void Statistics::save_data(const string& path){
	ofstream fs(path.c_str(), std::ofstream::out | std::ofstream::binary);
	byte s = 0;
	long sum = 0;

	FILE* fout;

	fout = fopen(path.c_str(), "wb");

	for(int ib=0;ib<this->m_bblocks;ib++)
		for(int ig=0;ig<this->m_gblocks;ig++)
			for(int ir=0;ir<this->m_rblocks;ir++)
				for(int ix=0;ix<this->m_xblocks;ix++)
					for(int iy=0;iy<this->m_yblocks;iy++)
						for(int ia=0;ia<this->m_ablocks;ia++)
						{
							Vec6i index = this->get_index(ib,ig,ir,ix,iy,ia);
							s = SIGMOID((double)this->m_data.at<int>(index)/this->m_stat_count)*255;
							sum += s;
							fwrite(&s, 1, 1, fout);
						}
	fclose(fout);

	cout<<"file wrote sum = "<<sum<<endl;
}

bool Statistics::read_data(const string& path){
	FILE* fin = fopen(path.c_str(), "rb");
	if(!fin) return false;
	byte t;
	long sum = 0;

	cout<<this->m_bblocks<<endl;
	cout<<this->m_gblocks<<endl;
	cout<<this->m_rblocks<<endl;
	cout<<this->m_xblocks<<endl;
	cout<<this->m_yblocks<<endl;
	cout<<this->m_ablocks<<endl;
	for(int ib=0;ib<this->m_bblocks;ib++)
		for(int ig=0;ig<this->m_gblocks;ig++)
			for(int ir=0;ir<this->m_rblocks;ir++)
				for(int ix=0;ix<this->m_xblocks;ix++)
					for(int iy=0;iy<this->m_yblocks;iy++)
						for(int ia=0;ia<this->m_ablocks;ia++)
						{
							Vec6i index = this->get_index(ib,ig,ir,ix,iy,ia);
							fread(&t, 1, 1, fin);
							sum += t;
							this->m_data.at<int>(index) = t;
						}
	fclose(fin);

	cout<<"file read sum = "<<sum<<endl;

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

	Vec6i index;
	for(int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{
			byte label = profile.at<Vec4b>(i,j)[3];

			B = image.at<Vec3b>(i,j)[0];
			G = image.at<Vec3b>(i,j)[1];
			R = image.at<Vec3b>(i,j)[2];

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

	int a_b = this->get_aspect_block(aspect);

	byte G,B,R;

	for(int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{
			B = image.at<Vec3b>(i,j)[0];
			G = image.at<Vec3b>(i,j)[1];
			R = image.at<Vec3b>(i,j)[2];

			int x_b = j*this->m_xblocks/width;
			int y_b = i*this->m_yblocks/height;
			int r_b = R*this->m_rblocks/256;
			int g_b = G*this->m_gblocks/256;
			int b_b = B*this->m_bblocks/256;

			Vec6i index = this->get_index(b_b,g_b,r_b,x_b,y_b,a_b);

			trimap.at<byte>(i,j) = this->m_data.at<int>(index);
		}
}


// ===========================================
Vec6i Statistics::get_index(int& b_b, int& g_b, int& r_b, int& x_b, int& y_b, int& a_b)
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
