/*
 * main.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */
#include "main.h"

bool parse_arg(int argc, char** argv){
	if(argc < 2)
	{
		return false;
	}
	int i=1;
	while(i<argc){
		string arg = argv[i];
		if(arg == "-i")
		{
			if(i+1 < argc)
				g_filename = argv[++i];
			else
				return false;
		}
		else
		{
			g_input_dir = arg;
			enable_gui = false;
		}
		++i;
	}
	return true;
}

void print_usage(int argc, char** argv){
	cout<<"usage: "<<argv[0]<<"[-i filename] [input_dir]"<<endl;
}

vector<string> get_files(const string& input_dir){
	vector<string> files;

	DIR *dpdf;
	struct dirent *epdf;

	dpdf = opendir(input_dir.c_str());
	if (dpdf != NULL){
	   while (epdf = readdir(dpdf)){
		   string filename = string(epdf->d_name);
		   if(filename.find(".jpg")!=filename.length() - 4)
			   continue;
		   if(filename.find("-profile") == std::string::npos)
			   files.push_back(filename);
	   }
	}
	return files;
}

bool matting(const string& input, const string& output, Mat* min, Mat* mout)
{
	Timer t;
	Matting M;

	double read_file_cost, matting_cost, write_file_cost;

	t.restart();

	*min = imread(input, CV_LOAD_IMAGE_COLOR);   // Read the file

	read_file_cost = t.getElapsedMilliseconds();

	if(! min->data )                              // Check for invalid input
	{
		cout << " ! Error ! Could not open or find the image" << std::endl ;
		return false;
	}

	t.restart();

	M.mat(*min, *mout);	// do matting

	matting_cost = t.getElapsedMilliseconds();

	t.restart();

	imwrite(output, *mout);

	write_file_cost = t.getElapsedMilliseconds();

	cout<<"Matting "<<input<<" to "<<output
		<<" size = "<<min->rows<<"x"<<min->cols
		<<" read = "<<read_file_cost<<"ms"<<" mat =  "<<matting_cost<<"ms write = "<<write_file_cost<<"ms"<<endl;

	return true;
}

void run_batch(const string& input_dir)
{
	vector<string> files = get_files(input_dir);

	Mat min, mout;
	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = input_dir + "/" + *it;
		string output_filename = filename.substr(0, filename.find_last_of('.')) + "-profile.jpg";
		string training_filename = input_dir + "/" + output_filename;

		matting(filename, output_filename, &min, &mout);
	}
}

int main(int argc, char** argv){

	if(!parse_arg(argc, argv)){
		print_usage(argc, argv);
		return 0;
	}

	if(g_input_dir.length() != 0)
	{
		run_batch(g_input_dir);
		return 0;
	}
	else
	{
		Mat min, mout;

		if(matting(g_filename, g_filename + "-p.jpg", &min, &mout))
		{
			float ratio = (float)mout.rows / mout.cols;
			int rows = mout.rows > 600 ? 600 : mout.rows;
			int cols = rows/ratio;
			cv::Mat image;
			cv::resize(mout,image, Size(), (float)rows/mout.rows, (float)cols/mout.cols);
			cv::namedWindow( g_filename.c_str(), CV_WINDOW_AUTOSIZE );
			cv::imshow( g_filename.c_str(), image );

			waitKey(0);                                          // Wait for a keystroke in the window
		}
	}

	return 0;
}
