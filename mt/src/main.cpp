/*
 * main.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include <Matting.h>

void print_usage(int argc, char** argv){
	cout<<"usage: "<<argv[0]<<" input_dir"<<endl;
}

vector<string> get_files(const char* input_dir){
	vector<string> files;

	DIR *dpdf;
	struct dirent *epdf;

	dpdf = opendir(input_dir);
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

int main(int argc, char** argv){

	if(argc < 2)
	{
		print_usage(argc, argv);
		return 0;
	}

	vector<string> files = get_files(argv[1]);

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = *it;
		string output_filename = filename.substr(0, filename.find_last_of('.')) + "-profile.jpg";
		string training_filename = string(argv[1]) + "/" + output_filename;

		Matting mat;
		cout<<"input = "<<filename<<" | training = " << training_filename << " | output =  "<<output_filename<<" | result = "<<mat.mat()<<endl;
	}

	return 0;
}
