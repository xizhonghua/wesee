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

void run_batch(const string& input_dir)
{
	vector<string> files = get_files(input_dir);

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = *it;
		string output_filename = filename.substr(0, filename.find_last_of('.')) + "-profile.jpg";
		string training_filename = input_dir + "/" + output_filename;

		Matting M;
		Mat min, mout;
		M.mat(min, mout);

		cout<<"input = "<<filename<<" | training = " << training_filename << " | output =  "<<output_filename<<endl;
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
		Mat image;
		image = imread(g_filename, CV_LOAD_IMAGE_COLOR);   // Read the file

		if(! image.data )                              // Check for invalid input
		{
			cout <<  "Could not open or find the image" << std::endl ;
			return -1;
		}

		namedWindow( "Display window", CV_WINDOW_AUTOSIZE );// Create a window for display.
		imshow( "Display window", image );                   // Show our image inside it.

		waitKey(0);                                          // Wait for a keystroke in the window
	}

	return 0;
}
