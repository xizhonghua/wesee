/*
 * main.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */
#include "main.h"

bool parse_arg(int argc, char** argv){
	if(argc < 2) {
		return false;
	}
	int i=1;
	while(i<argc){
		string arg = argv[i];
		if(arg == "-m"){
			if(i+1 < argc){
				g_setting.matting_filename = argv[++i];
				g_setting.matting_mode = true;
			}
			else{
				return false;
			}
		}
		else if(arg == "-t"){
			if(i+1 < argc){
				g_setting.training_filename = argv[++i];
				g_setting.training_mode = true;
			}else{
				return false;
			}
		}
		else if(arg == "-ta"){
			if(i+1 < argc) {
				g_setting.training_dir = argv[++i];
				g_setting.training_batch_mode = true;
			}
		}
		else
		{
			g_setting.input_dir = arg;
			g_setting.enable_gui = false;
		}
		++i;
	}
	return true;
}

void print_usage(int argc, char** argv){
	cout<<"usage: "<<argv[0]<<" [-m filename] [-t filename] [-ta input_dir] [input_dir]"<<endl;
	cout<<"\t-m filename: mat single image"<<endl;
	cout<<"\t-t filename: train single image"<<endl;
	cout<<"\t-ta input_dir: train entire directory"<<endl;
	cout<<"\tinput_dir: mat entire directory"<<endl;
}

string get_training_profile_name(const string& input){
	return input.substr(0, input.find_last_of('.')) + "-profile.jpg";
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

	cout<<"[Matting] from "<<input<<" to "<<output
		<<" size = "<<min->rows<<"x"<<min->cols
		<<" read = "<<read_file_cost<<"ms"<<" mat =  "<<matting_cost<<"ms write = "<<write_file_cost<<"ms"<<endl;

	return true;
}

bool training(const string& input, const string& profile)
{
	Timer t;
	Mat img_org, img_profile;
	double read_file_cost, training_cost;
	t.restart();

	img_org = imread(input, CV_LOAD_IMAGE_COLOR);
	if(!img_org.data)
	{
		cout << " ! Error ! Could not open or find the image" <<input<< std::endl ;
		return false;
	}

	img_profile = imread(profile, CV_LOAD_IMAGE_COLOR);
	if(!img_profile.data)
	{
		cout<< " ! Error ! Could not open or find the image "<<profile<< std::endl ;
		return false;
	}

	read_file_cost = t.getElapsedMilliseconds();

	t.restart();
	Matting M;
	M.train(img_org, img_profile);
	training_cost = t.getElapsedMilliseconds();

	cout<<"[Training] from "<<input<<" & "<<profile
		<<" size = "<<img_org.rows<<"x"<<img_org.cols
		<<" read = "<<read_file_cost<<"ms"<<" train =  "<<training_cost<<"ms"<<endl;

	return true;
}

void run_batch(const string& input_dir)
{
	vector<string> files = get_files(input_dir);

	Mat min, mout;
	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = input_dir + "/" + *it;
		string output_filename = get_training_profile_name(filename);
		string training_filename = input_dir + "/" + output_filename;

		matting(filename, output_filename, &min, &mout);
	}
}

void train_batch(const string& input_dir)
{
	vector<string> files = get_files(input_dir);

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = input_dir + "/" + *it;
		string training_filename = get_training_profile_name(filename);

		training(filename, training_filename);
	}

	Matting::dump_training_results();
}

int main(int argc, char** argv){

	if(!parse_arg(argc, argv)){
		print_usage(argc, argv);
		return 0;
	}

	if(g_setting.matting_batch_mode)
	{
		run_batch(g_setting.input_dir);
	}

	if(g_setting.training_batch_mode)
	{
		train_batch(g_setting.training_dir);
	}

	if(g_setting.training_mode)
	{
		training(g_setting.training_filename, get_training_profile_name(g_setting.training_filename));
		Matting::dump_training_results();
	}

	if(g_setting.matting_mode)
	{
		Mat min, mout;

		if(matting(g_setting.matting_filename, g_setting.matting_filename + "-p.jpg", &min, &mout))
		{
			float ratio = (float)mout.rows / mout.cols;
			int rows = mout.rows > 600 ? 600 : mout.rows;
			int cols = rows/ratio;
			cv::Mat image;
			cv::resize(mout,image, Size(), (float)rows/mout.rows, (float)cols/mout.cols);
			cv::namedWindow(g_setting.matting_filename.c_str(), CV_WINDOW_AUTOSIZE );
			cv::imshow( g_setting.matting_filename.c_str(), image );

			waitKey(0);                                          // Wait for a keystroke in the window
		}
	}

	return 0;
}
