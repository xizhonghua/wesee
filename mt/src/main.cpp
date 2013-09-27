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
		else if(arg == "-mr" || arg == "--max-refine") {
			if(i+1 < argc && argv[i+1][0] != '-') {
				stringstream ss(argv[++i]);
				ss>>g_setting.max_refine_iterations;
				g_setting.max_refine_iterations = std::max(g_setting.max_refine_iterations, 1);
				g_setting.max_refine_iterations = std::min(g_setting.max_refine_iterations, 100);
			}
		}
		else if(arg == "-op") {
			g_setting.output_prediction = true;
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
				if(i+1 < argc && argv[i+1][0] != '-'){
					stringstream ss(argv[++i]);
					ss>>g_setting.max_training_images;
				}
			}else {
				return false;
			}
		}
		else if(arg == "-td") {
			if(i+1 < argc ) {
				g_setting.training_database_filename = argv[++i];
			} else {
				return false;
			}
		}
		else if(arg == "-e") {
			g_setting.enable_evaluation = true;
		}
		else if(arg == "-g") {
			g_setting.enable_gui = false;
		}
		else if(arg == "-ev") {
			if(i+2 < argc){
				g_setting.evaluation_mode = true;
				g_setting.profile_filename = argv[++i];
				g_setting.profile_ground_truth_filename = argv[++i];
			}
			else {
				return false;
			}
		}
		else if(arg == "-h") {
			return false;
		}
		else if(arg == "-resize") {
			g_setting.resize_mode = true;
			if(i+1 < argc) {
				g_setting.resize_filename = argv[++i];
			}else{
				return false;
			}
			if(i+1 < argc && argv[i+1][0] != '-') {
				stringstream ss(argv[++i]);
				ss>>g_setting.resize_long_edge;
			}
			else{
				g_setting.resize_long_edge = 240;
			}
		}
		else if(arg == "-test") {
			if(i+2 < argc){
				g_setting.test_mode = true;
				g_setting.test_profilename = argv[++i];
				g_setting.test_filename = argv[++i];

			}
			else {
				return false;
			}

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
	cout<<"usage: "<<argv[0]<<" [Options]"<<endl;
	cout<<"examples:"<<endl;
	cout<<"\t"<<argv[0]<<" -e -m 001.jpg "<<"# matting & evaluating 001.jpg with 001-profile.jpg"<<endl;
	cout<<endl;
	cout<<"options:"<<endl;
	cout<<"\t[-g]: no gui"<<endl;
	cout<<"\t[-e]: enable evaluation, will NOT save result to file"<<endl;
	cout<<"\t[-m filename]: mat single image"<<endl;
	cout<<"\t[-mr iterations]: max refine iterations"<<endl;
	cout<<"\t[-ev profile ground_truth]: evaluation profile with ground_truth"<<endl;
	cout<<"\t[-ta input_dir]: train entire directory"<<endl;
	cout<<"\t[-td filename]: use custom training database"<<endl;
	cout<<"\t[-op]: output predition map"<<endl;
	cout<<"\t[-resize filename [long_edge=360]]: resize the image"<<endl;
	cout<<"\t[input_dir]: mat entire directory"<<endl;
	cout<<"\t[-h]: show this message"<<endl;
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
		   if(filename.find(".jpg")!=filename.length() - 4
		   && filename.find(".JPG")!=filename.length() - 4
		   && filename.find(".jpeg")!=filename.length() - 5
		   && filename.find(".JPEG")!=filename.length() - 5)
			   continue;
		   if(filename.find("-profile") == std::string::npos)
			   files.push_back(filename);
	   }
	}
	return files;
}

bool training(Statistics& stat, const string& input, const string& profile, long& pixel_count)
{
	Timer t;
	Mat img_org, img_profile;
	double read_file_cost, training_cost;
	t.restart();

	img_org = imread(input);
	if(!img_org.data)
	{
		cerr << " ! Error ! Could not open or find the image" <<input<< std::endl ;
		return false;
	}

	img_profile =  MatHelper::read_image_ch(profile, 3);
	if(!img_profile.data)
	{
		cerr << " ! Error ! Could not open or find the image "<<profile<< std::endl ;
		return false;
	}

	img_org = MatHelper::resize(img_org, LONG_EDGE_TRAIN);
	img_profile = MatHelper::resize(img_profile, LONG_EDGE_TRAIN);

	read_file_cost = t.getElapsedMilliseconds();

	t.restart();

	stat.stat(img_org, img_profile);

	training_cost = t.getElapsedMilliseconds();

	cerr <<"[Training] from "<<input<<" & "<<profile
		<<" size = "<<img_org.rows<<"x"<<img_org.cols
		<<" read = "<<read_file_cost<<"ms"<<" train =  "<<training_cost<<"ms"<<endl;

	pixel_count += img_org.rows * img_org.cols;

	return true;
}

void run_batch(const string& input_dir)
{

	cerr<<"========================== WESEE ============================"<<endl;

	Statistics stat;

	Timer tt;

	if(stat.read_data(g_setting.training_database_filename)){
		cerr<<"- Training data loaded from " << g_setting.training_database_filename <<" in "<<tt.getElapsedMilliseconds()<<"ms"<<endl;
	}
	else {
		cerr<<"! Error ! Can't open training data : "<<g_setting.training_database_filename<<endl;
		return;
	}

	cerr<<"- Batch started"<<endl;

	vector<string> files = get_files(input_dir);

	cerr<<"- "<<files.size()<<" images(s) got"<<endl;

	int jobid = 0;

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		Timer t;

		try {
			const string filename = input_dir + "/" + *it;
			const string profile_filename = get_profile_name(filename);
			string output_filename = g_setting.output_dir + "/" + *it;

			cerr<<"["<<++jobid<<"/"<<files.size()<<"] [Matting] " + filename + "...";

			Mat ori = imread(filename);

			if(!ori.data) {
				cerr<<"! Error ! can't open or read image file "<<filename<<"!"<<endl;
				continue;
			}

			Mat output, predict_raw, predict_drawing;

			Matting::mat(stat, ori, output, predict_raw, predict_drawing);

			imwrite(output_filename, output);

			cerr<<" done Time = "<<t.getElapsedMilliseconds()<<" ms"<<" | Result saved to "<<output_filename<<endl;

			if(g_setting.output_prediction){
				imwrite(filename + ".predict.png", predict_drawing);
				imwrite(filename + ".predict-raw.png", predict_raw);

				cerr<<"- Prediction file saved to " + filename + ".predict.png"<<endl;
			}

			if(g_setting.enable_evaluation) {
				evaluate(output_filename, profile_filename);
			}
		}
		catch(int e) {
			//
		}
	}

	cerr<<"- Batch done "<<files.size()<<" image(s) processed!"<<endl;
	cerr<<"- Total time : "<<tt.getElapsedMilliseconds()/1000<<"s"<<" Average time:"<<(files.size()==0?0:tt.getElapsedMilliseconds()/files.size())<<"ms"<<endl;
}


void train_batch(const string& input_dir)
{

	Timer t;

	Statistics stat;

	vector<string> files = get_files(input_dir);

	int trained = 0;
	long pixel_count = 0;

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = input_dir + "/" + *it;
		string training_filename = get_profile_name(filename);

		cerr<<"["<<trained<<"/"<<(g_setting.max_training_images == 0 ? files.size() : g_setting.max_training_images)<<"] ";
		trained += training(stat, filename, training_filename, pixel_count);

		if(g_setting.max_training_images != 0 && g_setting.max_training_images <= trained) break;
	}

	double training_cost = t.getElapsedMilliseconds();

	stat.save_data(g_setting.training_database_filename);
	cout<<"- Training data saved to "<<g_setting.training_database_filename<<endl;

	cout<<trained<<" images with "<< pixel_count <<" pixels trained in "<<training_cost<<"ms"<<endl;
}

void evaluate(const string& profile, const string& ground_truth){
	Mat p = cv::imread(profile, cv::IMREAD_UNCHANGED);
	Mat g = cv::imread(ground_truth, cv::IMREAD_UNCHANGED);
	vector<Mat> ch;
	cv::split(p, ch);
	p = ch.back();
	ch.clear();
	split(g, ch);
	g = ch.back();
	if(ch .size() != 4) {
		cerr << "! Warning ! "<<ground_truth<<" doest not contain alpha channel!"<<endl;
		g = 255 - g;
	}
	Matting m;
	double score = m.evaluate(g, p);
	printf("- Score = %.4lf | Profile = %s | Ground_truth = %s\n", score, profile.c_str(), ground_truth.c_str());
}

void resize(const string& filename, const int long_edge){
	Mat img = cv::imread(filename, cv::IMREAD_UNCHANGED);
	Mat out = MatHelper::resize(img, long_edge);
	string out_filename = filename;
	out_filename.insert(out_filename.find_last_of('.'), "-s");
	out_filename.erase(out_filename.find_last_of('.'));
	out_filename += ".png";

	vector<int> params;
	params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	params.push_back(9);
	cv::imwrite(out_filename, out);

	printf("resized %s from %dx%d to %dx%d, output saved to %s\n", filename.c_str(), img.cols, img.rows, out.cols, out.rows, out_filename.c_str());
}


void salient(const string& salient_path, const string& input_path){


	Mat salient = imread(salient_path, IMREAD_UNCHANGED);
	Mat input = imread(input_path);
	//cerr<<"salient file:"<<salient_path<<" size = "<<salient.size()<<endl;
	//cerr<<"input_path:"<<input_path<<" size = "<<input.size()<<endl;
	int org_width = input.cols;
	int org_height = input.rows;
	input = MatHelper::resize(input, salient.cols, salient.rows);

//	imwrite(salient_path + ".s.png", MatHelper::resize(salient, org_width, org_height));
//	evaluate(salient_path + ".s.png", get_profile_name(input_path));
//	return;

	Mat mask;
	mask.create(input.size(), CV_8UC1);
	mask = cv::GC_PR_FGD;

	Mat bgdModel;
	Mat fgdModel;

	int width = salient.cols;
	int height = salient.rows;

	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{
//			double s = (double)rand()/RAND_MAX;
//			if(s>0.5) continue;
			int value = salient.at<byte>(i,j);
			mask.at<byte>(i,j) = (value>127) ? cv::GC_PR_FGD : cv::GC_PR_BGD;
		}

	Rect boundRect;
	cv::grabCut(input, mask, boundRect, bgdModel, fgdModel, g_setting.max_refine_iterations, cv::GC_INIT_WITH_MASK);

	Mat seg = Mat(input.size(), CV_8UC1, Scalar(255));
	Mat output;
	seg.copyTo(output, mask & 1);

	output = MatHelper::resize(output, org_width, org_height);

	string output_path = salient_path + ".salient-out.png";
	imwrite(output_path, output);

	evaluate(output_path, get_profile_name(input_path));
}

int main(int argc, char** argv){
	if(!parse_arg(argc, argv)){
		print_usage(argc, argv);
		return 0;
	}

	if(g_setting.evaluation_mode){
		evaluate(g_setting.profile_filename, g_setting.profile_ground_truth_filename);
		return 0;
	}

	if(g_setting.resize_mode){
		resize(g_setting.resize_filename, g_setting.resize_long_edge);
		return 0;
	}

	if(g_setting.matting_batch_mode)
	{
		run_batch(g_setting.input_dir);
		return 0;
	}

	if(g_setting.training_batch_mode)
	{
		train_batch(g_setting.training_dir);
		return 0;
	}

	if(g_setting.test_mode){
		salient(g_setting.test_profilename, g_setting.test_filename);
	}

	return 0;
}
