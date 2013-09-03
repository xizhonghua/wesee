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
		   if(filename.find(".jpg")!=filename.length() - 4)
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
	cerr<<"- Batch starting"<<endl;

	Statistics stat;

	Timer t;

	if(stat.read_data(g_setting.training_database_filename)){
		cerr<<"- Training data loaded from " << g_setting.training_database_filename <<" in "<<t.getElapsedMilliseconds()<<"ms"<<endl;
	}
	else {
		cerr<<"! Error ! Can't open training data."<<endl;
		return;
	}
	vector<string> files = get_files(input_dir);

	cerr<<"- "<<files.size()<<" job(s) got"<<endl;

	int jobid = 0;

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		try {
			const string filename = input_dir + "/" + *it;
			const string profile_filename = get_profile_name(filename);
			string output_filename = get_profile_name(*it);

			cerr<<"["<<++jobid<<"/"<<files.size()<<"] [Matting] " + filename + "..."<<endl;

			Mat ori = imread(filename);

			if(ori.type() != CV_8UC3) {
				cerr<<"! Error ! unsupported data type = "<<ori.type()<<"!"<<endl;
				continue;
			}
			Mat input = MatHelper::resize(ori, LONG_EDGE_PX);
			Mat predict = Mat::zeros(ori.rows, ori.cols, CV_8UC1);

			stat.predict(ori, predict);

			Mat predict_s = MatHelper::resize(predict, LONG_EDGE_PX);

			//cv::fastNlMeansDenoising(predict_s, predict_s, 11);
			//blur( predict_s, predict_s, Size(11,11) );
			medianBlur(predict_s, predict_s, 5);

			Mat threshold_output;
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;

			int thresh = 70;
			int max_thresh = 255;
			RNG rng(12345);

			/// Detect edges using Threshold
			threshold( predict_s, threshold_output, thresh, 255, THRESH_BINARY );
			/// Find contours
			findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

			/// Approximate contours to polygons + get bounding rects and circles
			vector<vector<Point> > contours_poly( contours.size() );
			vector<Rect> boundRect( contours.size() );
			vector<Point2f>center( contours.size() );
			vector<float>radius( contours.size() );

			Rect bestBoundRect;
			vector<Point> betContourPoly;
			int best_index;
			for( int i = 0; i < contours.size(); i++ )
			{
				approxPolyDP( Mat(contours[i]), contours_poly[i], 1, true );
				boundRect[i] = boundingRect( Mat(contours_poly[i]) );
				if(boundRect[i].area() > bestBoundRect.area())  {
					bestBoundRect = boundRect[i];
					betContourPoly = contours_poly[i];
					best_index = i;
				}
			}

			bestBoundRect.x -= bestBoundRect.width * 0.15;
			bestBoundRect.x = std::max(bestBoundRect.x, 0);
			bestBoundRect.width *= 1.3;
			bestBoundRect.width = std::min(predict_s.cols - bestBoundRect.x, bestBoundRect.width);

			bestBoundRect.y -= bestBoundRect.height * 0.15;
			bestBoundRect.y = std::max(bestBoundRect.y, 0);
			bestBoundRect.height *= 1.3;
			bestBoundRect.height = std::min(predict_s.rows - bestBoundRect.y, bestBoundRect.height);

			if(g_setting.output_prediction)
			{
				Scalar WHITE = Scalar( 255, 255, 255 );
				Scalar BLACK = Scalar( 0, 0, 0 );

				// ===================================================
				// drawing
				// ===================================================
				Mat drawing = predict_s.clone();

				rectangle( drawing, bestBoundRect.tl(), bestBoundRect.br(), WHITE, 2, 8, 0 );
				drawContours( drawing, contours_poly, best_index, WHITE, 1, 8, vector<Vec4i>(), 0, Point() );

				imwrite(filename + ".predict.png", drawing);
				imwrite(filename + ".predict-raw.png", predict);

				cerr<<"- Prediction file saved to " + filename + ".predict.png"<<endl;
			}

			Mat result;
			double cost = grabCut(ori, input, predict_s, bestBoundRect, contours_poly[best_index], result);

			Mat output = MatHelper::resize(result, ori.cols, ori.rows);
			medianBlur(output, output, 7);

			cv::threshold(output, output, 128, 255, CV_THRESH_BINARY);

			output = 255 - output;

			string output_profile_name = profile_filename.substr(0, profile_filename.find_last_of(".")) + ".png";
			imwrite(output_profile_name, output);

			cerr<<"- Time = "<<cost<<" ms"<<" | Result saved to "<<output_profile_name<<endl;

			if(g_setting.enable_evaluation) {
				evaluate(output_profile_name, profile_filename);
			}
		}
		catch(int e) {
			//
		}
	}

	cerr<<"- Batch done"<<endl;
	cerr<<"========================== WESEE ============================"<<endl;
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

double grabCut(const Mat& ori, const Mat& min, const Mat& trimap, const Rect& boundRect, const vector<Point>& contour, Mat& output){
	Timer t;
	t.restart();

	Mat input, mask;
	mask.create(min.size(), CV_8UC1);
	mask = cv::GC_BGD;

	Mat bgdModel;
	Mat fgdModel;

	int width = min.cols;
	int height = min.rows;

	mask(boundRect) = cv::GC_PR_FGD;

	for(int j=boundRect.x;j<boundRect.x + boundRect.width;j++)
		for(int i=boundRect.y;i<boundRect.y + boundRect.height;i++)
		{
			int value = trimap.at<byte>(i,j);

			double dist = cv::pointPolygonTest(contour, Point2i(j,i), true);

			if(dist > 0){
				// inside the contour
				if(value >= 240){
					mask.at<byte>(i,j) = cv::GC_FGD;
				}
				else if(value <= 12)
				{
					mask.at<byte>(i,j) = cv::GC_PR_BGD;
				}
			}
			if(dist < 0)
			{
				if(value <= 12)
					mask.at<byte>(i,j) = cv::GC_BGD;
				else if(value <= 30)
					mask.at<byte>(i,j) = cv::GC_PR_BGD;
			}
		}

	cv::cvtColor(min, min, CV_BGR2XYZ);

	cv::grabCut(min, mask, boundRect, bgdModel, fgdModel, g_setting.max_refine_iterations, cv::GC_INIT_WITH_MASK);

	Mat seg = Mat(min.size(), CV_8UC1);
	seg = 255;

	seg.copyTo(output, mask & 1);

	double cost = t.getElapsedMilliseconds();

	output = MatHelper::resize(output, ori.cols,  ori.rows);

	return cost;
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
