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
		else if(arg == "-e") {
			g_setting.enable_evaluation = true;
		}
		else
		{
			g_setting.matting_batch_mode=true;
			g_setting.input_dir = arg;
			g_setting.enable_gui = false;
		}
		++i;
	}
	return true;
}

void print_usage(int argc, char** argv){
	cout<<"usage: "<<argv[0]<<" [-e] [-m filename] [-t filename] [-ta input_dir] [input_dir]"<<endl;
	cout<<"\t-e enable evaluation, will NOT save result to file"<<endl;
	cout<<"\t-m filename: mat single image"<<endl;
	cout<<"\t-t filename: train single image"<<endl;
	cout<<"\t-ta input_dir: train entire directory"<<endl;
	cout<<"\tinput_dir: mat entire directory"<<endl;
}

string get_profile_name(const string& input){
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

bool matting(const string& input, const string& output, Mat* min, Mat* mout, const string* ground_truth = NULL, double* score = NULL)
{
	Timer t;
	Matting M;

	double read_file_cost = 0, matting_cost = 0, write_file_cost = 0, evaulation_cost = 0;

	t.restart();

	*min = imread(input, CV_LOAD_IMAGE_COLOR);   // Read the file

	read_file_cost = t.getElapsedMilliseconds();

	if(! min->data )                              // Check for invalid input
	{
		cout << " ! Error ! Could not open or find the image" << std::endl ;
		return false;
	}

	// =========================================
	// matting
	// =========================================

	t.restart();

	M.mat(*min, *mout);

	matting_cost = t.getElapsedMilliseconds();

	// =========================================
	// save result if no evaluation switch specified
	// =========================================
	if(!g_setting.enable_evaluation){
		t.restart();

		imwrite(output, *mout);

		write_file_cost = t.getElapsedMilliseconds();
	}

	cout<<"[Matting] from "<<input<<" to "<<output
		<<" size = "<<min->rows<<"x"<<min->cols
		<<" read = "<<read_file_cost<<"ms"<<" mat =  "<<matting_cost<<"ms write = "<<write_file_cost<<"ms"<<endl;

	if(g_setting.enable_evaluation && ground_truth){

		// ==========================================
		// evaluation
		// ==========================================

		t.restart();

		Mat img_ground_truth = imread(*ground_truth, CV_LOAD_IMAGE_COLOR);

		read_file_cost = t.getElapsedMilliseconds();

		if(!img_ground_truth.data){
			cout << " ! Error ! Could not open or find the ground truth image " << *ground_truth<< std::endl ;
			return false;
		}

		double p = Matting::evaluate(img_ground_truth, *mout);

		if(score) *score = p;

		evaulation_cost = t.getElapsedMilliseconds() - read_file_cost;

		cout<<"[Evaluation] with "<<*ground_truth<<" score = "<<*score<<endl;
	}

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
	double total_score = 0.0;


	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string filename = input_dir + "/" + *it;
		const string profile_filename = get_profile_name(filename);
		string output_filename = get_profile_name(*it);
		double score = 0.0;

		matting(filename, output_filename, &min, &mout, &profile_filename, &score);
		total_score += score;
	}

	if(g_setting.enable_evaluation)
		cout<<"Average score = "<< total_score / files.size()<<endl;
}

void train_batch(const string& input_dir)
{
	vector<string> files = get_files(input_dir);

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = input_dir + "/" + *it;
		string training_filename = get_profile_name(filename);

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
		training(g_setting.training_filename, get_profile_name(g_setting.training_filename));
		Matting::dump_training_results();
	}

	if(g_setting.matting_mode)
	{
		Mat min, mout;
		double score;

		string profile_name = get_profile_name(g_setting.matting_filename);

		if(matting(g_setting.matting_filename, "tmp-matting.jpg", &min, &mout, &profile_name, &score))
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
