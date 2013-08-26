/*
 * main.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: zhonghua
 */
#include "main.h"

Image<Color> *displayImage;
GrabCut* gc = NULL;

// Some state variables for the UI
Real xstart, ystart, xend, yend;
bool box = false;
bool initialized = false;
bool is_left = false;
bool is_right = false;
bool refining = false;
bool showMask = false;
int displayType = 0;
int edits = 0;


// used to evaluation
Mat img_ground_truth;

void init()
{
	//set the background color to black (RGBA)
	glClearColor(0.0,0.0,0.0,0.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

// Draw the image and paint the chosen mask over the top.
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	gc->display(displayType);

	if (showMask)
	{
		gc->overlayAlpha();
	}

	if (box)
	{
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex2d( xstart, ystart );
		glVertex2d( xstart, yend );
		glVertex2d( xend, yend );
		glVertex2d( xend, ystart );
		glEnd();
	}

	glFlush();
	glutSwapBuffers();
}

void idle()
{
	int changed = 0;

	if (refining)
		{
		changed = gc->refineOnce();
		glutPostRedisplay();
		}

	if (!changed)
		{
		refining = false;
		glutIdleFunc(NULL);
		}
}

void mouse(int button, int state, int x, int y)
{
	y = displayImage->height() - y;

	switch(button)
	{
	case GLUT_LEFT_BUTTON:
		if (state==GLUT_DOWN)
		{
			is_left = true;

			if (!initialized)
			{
				xstart = x; ystart = y;
				box = true;
			}
		}

		if( state==GLUT_UP )
		{
			is_left = false;

			if( initialized )
			{
				gc->refineOnce();
				glutPostRedisplay();
			}

			else
			{
				xend = x; yend = y;
				gc->initialize(xstart, ystart, xend, yend);
				gc->fitGMMs();
				box = false;
				initialized = true;
				showMask = true;
				glutPostRedisplay();
			}
		}
		break;

	case GLUT_RIGHT_BUTTON:
		if( state==GLUT_DOWN )
		{
			is_right = true;
		}
		if( state==GLUT_UP )
		{
			is_right = false;

			if( initialized )
			{
				gc->refineOnce();
				glutPostRedisplay();
			}
		}
		break;

	default:
		break;
	}
}

void motion(int x, int y)
{
	y = displayImage->height() - y;

	if( box == true )
	{
		xend = x; yend = y;
		glutPostRedisplay();
	}

	if( initialized )
	{
		if( is_left )
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapForeground);

		if( is_right )
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapBackground);

		glutPostRedisplay();
	}
}

double evaluation(Mat& im){
	if(!img_ground_truth.data) {
		cout<<"ground truth not found"<<endl;
		return 0;
	}

	vector<Mat> ch;
	split(img_ground_truth, ch);

	double score = Matting::evaluate(ch[ch.size()-1], im);

	Mat ground_truth_to_display = MatHelper::resize(img_ground_truth, LONG_EDGE_PX);
	split(ground_truth_to_display, ch);

	Mat resultToShow = MatHelper::resize(im, LONG_EDGE_PX);

	if(!g_setting.enable_gui) return score;

	cv::namedWindow("matting result", CV_WINDOW_AUTOSIZE );
	cv::imshow("matting result", resultToShow);
	cv::moveWindow("matting result", 100 + ground_truth_to_display.cols + 20, 100);
	cv::namedWindow("ground truth", CV_WINDOW_AUTOSIZE );
	cv::imshow("ground truth", ch[3]);
	cv::moveWindow("ground truth", 100 + (ground_truth_to_display.cols + 20)*2, 100);

	cerr<<"score "<<score<<endl;

	return score;
}

void keyboard(unsigned char key, int x, int y)
{
	y = displayImage->height() - y;
	Mat result,output;

	switch (key)
	{
	case 'q':case 'Q':				// quit
		exit(0);
		break;

	case ' ':						// space bar show/hide alpha mask
		showMask = !showMask;
		break;

	case '1': case 'i': case 'I':	// choose the image
		displayType = 0;
		break;

	case '2': case 'g': case 'G':	// choose GMM index mask
		displayType = 1;
		break;

	case '3': case 'n': case 'N':	// choose N-Link mask
		displayType = 2;
		break;

	case '4': case 't': case 'T':	// choose T-Link mask
		displayType = 3;
		break;

	case 'r':						// run GrabCut refinement
		refining = true;
		glutIdleFunc(idle);
		break;

	case 'o':						// run one step of GrabCut refinement
		gc->refineOnce();
		glutPostRedisplay();
		break;

	case 'l':
		gc->fitGMMs();			// rerun the Orchard-Bowman GMM clustering
		glutPostRedisplay();
		break;

	case 'e':					// evaluation
		gc->getSegmentationResult(&result);
		output = MatHelper::resize(result, img_ground_truth.cols,  img_ground_truth.rows);
		evaluation(output);
		break;

	case 27:
		refining = false;
		glutIdleFunc(NULL);

	default:
		break;
	}

	glutPostRedisplay();
}

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
			g_setting.output_profile = true;
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
			}else {
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
	cout<<"\t[-t filename]: train single image"<<endl;
	cout<<"\t[-ev profile ground_truth]: evaluation profile with ground_truth"<<endl;
	cout<<"\t[-ta input_dir]: train entire directory"<<endl;
	cout<<"\t[-op]: output profile, should use with [-m]"<<endl;
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

		img_ground_truth = imread(*ground_truth, CV_LOAD_IMAGE_COLOR);

		read_file_cost = t.getElapsedMilliseconds();

		if(!img_ground_truth.data){
			cerr << " ! Error ! Could not open or find the ground truth image " << *ground_truth<< std::endl ;
			return false;
		}

		double p = Matting::evaluate(img_ground_truth, *mout);

		if(score) *score = p;

		evaulation_cost = t.getElapsedMilliseconds() - read_file_cost;

		cout<<"[Evaluation] with "<<*ground_truth<<" score = "<<*score<<endl;
	}

	return true;
}

bool training(Statistics& stat, const string& input, const string& profile)
{
	Timer t;
	Mat img_org, img_profile;
	double read_file_cost, training_cost;
	t.restart();

	img_org = imread(input, CV_LOAD_IMAGE_UNCHANGED);
	if(!img_org.data)
	{
		cerr << " ! Error ! Could not open or find the image" <<input<< std::endl ;
		return false;
	}

	img_profile = imread(profile, CV_LOAD_IMAGE_UNCHANGED);
	if(!img_profile.data)
	{
		cerr << " ! Error ! Could not open or find the image "<<profile<< std::endl ;
		return false;
	}

	read_file_cost = t.getElapsedMilliseconds();

	t.restart();

	stat.stat(img_org, img_profile);

	training_cost = t.getElapsedMilliseconds();

	cerr <<"[Training] from "<<input<<" & "<<profile
		<<" size = "<<img_org.rows<<"x"<<img_org.cols
		<<" read = "<<read_file_cost<<"ms"<<" train =  "<<training_cost<<"ms"<<endl;

	return true;
}

void run_batch(const string& input_dir)
{
	Statistics stat;

	Timer t;

	if(!stat.read_data("train.bin")){
		cerr<<"Can't open training data."<<endl;
	}
	else {
		cerr<<"Training data loaded in "<<t.getElapsedMilliseconds()<<"ms"<<endl;
	}

	vector<string> files = get_files(input_dir);

	cerr<<files.size()<<" job(s) got"<<endl;

	Mat min, mout;
	double total_score = 0.0;


	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{

		const string filename = input_dir + "/" + *it;
		const string profile_filename = get_profile_name(filename);
		string output_filename = get_profile_name(*it);

		cerr<<"processing " + filename + "..."<<endl;

		Mat ori = MatHelper::read_image(filename);
		Mat input = MatHelper::read_image(filename, LONG_EDGE_PX);
		Mat predict = Mat::zeros(input.rows, input.cols, CV_8UC1);

		stat.predict(input, predict);

		cout<<"predicted"<<endl;

		imwrite(filename + ".predict.png", predict);

		if(g_setting.enable_evaluation) {
			Mat ground_truth = imread(profile_filename, cv::IMREAD_UNCHANGED);
			//TODO
		}

		if(g_setting.output_profile) {
			//TODO
		}
	}
}

void train_batch(const string& input_dir)
{

	Timer t;

	Statistics stat;

	vector<string> files = get_files(input_dir);

	int trained = 0;

	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
	{
		const string& filename = input_dir + "/" + *it;
		string training_filename = get_profile_name(filename);

		trained += training(stat, filename, training_filename);
	}


	double training_cost = t.getElapsedMilliseconds();

	stat.save_data("train.bin");
	cout<<"training data saved"<<endl;

//	files = get_files("test");
//
//	for(vector<string>::const_iterator it = files.begin(); it != files.end(); ++ it)
//	{
//		const string& filename = "test/" + *it;
//
//		Mat input, output;
//		input = imread(filename, CV_LOAD_IMAGE_UNCHANGED);
//		if(!input.data) {
//			cerr<<"Can't open "<<filename<<endl;
//			continue;
//		}
//		output = Mat::zeros(input.rows, input.cols, CV_8UC1);
//		stat.predict(input, output);
//
//		string output_filename = filename + ".predit.png";
//		cout<<"dumping "<<output_filename<<endl;
//
//		cv::imwrite(output_filename, output);
//	}

	cout<<trained<<" image trained in "<<training_cost<<"ms"<<endl;
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
	Matting m;
	double score = m.evaluate(g, p);
	printf("score = %.4lf profile = %s ground_truth = %s\n", score, profile.c_str(), ground_truth.c_str());
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

double autoGrabCut(const Mat& min, Mat& output){
	Timer t;
	t.restart();

	int width = min.cols;
	int height = min.rows;


	const int PT = 20;
	const int CL = 1;
	const double PI = acos(-1);


	// portrait
	if(height > width) {
		gc->initialize(0.15*width, 0.01*height, 0.85*width, 0.98*height);
		gc->fitGMMs();

		// CLICK SOME FOREGROUND
		for(int i=3;i<PT-4;i++){
			int y = height*(double)i/PT;
			for(int j=-CL;j<=CL;j++){
				int x = width/2 + j*4;
				gc->setTrimap(x-2,y-2,x+2,y+2,TrimapForeground);
			}
		}

		// CLICK SOME BACKGROUND
		for(int i=0;i<PT;i++){
			int x = 0.2*width;
			int y = height*(double)i/PT;
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapBackground);

			x = 0.8*width;
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapBackground);
		}

	} else {
		// landscape
		gc->initialize(0.15*width, 0.01*height, 0.85*width, 0.98*height);
		gc->fitGMMs();



		// CLICK SOME FOREGROUND
		for(int i=6;i<PT-6;i++){
			int x = width*(double)i/PT;
			int y = height/2;
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapForeground);
		}

		// CLICK SOME FOREGROUND
		for(int i=3;i<PT-6;i++){
			int y = height*(double)i/PT;
			int x = width/2;
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapForeground);

		}

		// CLICK SOME BACKGROUND
		for(int i=0;i<PT;i++){
			int x = width*1/3*cos(i*PI/PT) + width/2;
			int y = height*1/3*sin(i*PI/PT) + height/2;
			gc->setTrimap(x-2,y-2,x+2,y+2,TrimapBackground);
		}
	}

	initialized = true;
	showMask = true;

	int max_refine_times = g_setting.max_refine_iterations;
	int changed = INT_MAX;

	while(changed > 5 && max_refine_times-- && t.getElapsedMilliseconds() < 2000){
		changed = gc->refineOnce();
	}

	double cost = t.getElapsedMilliseconds();

	Mat seg;
	gc->getSegmentationResult(&seg);

	output = MatHelper::resize(seg, img_ground_truth.cols,  img_ground_truth.rows);

	return cost;
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

	if(g_setting.matting_mode)
	{
		Mat min, mout;
		double score;

		string profile_name = get_profile_name(g_setting.matting_filename);

		Image<Color>* image = loadForOCV( g_setting.matting_filename, LONG_EDGE_PX, min);

		img_ground_truth = imread(profile_name, cv::IMREAD_UNCHANGED);

		if (image)
		{
			displayImage = image;

			gc = new GrabCut( image );

			if(g_setting.enable_gui) {
				glutInit(&argc,argv);
				glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

				glutInitWindowSize(displayImage->width(),displayImage->height());
				glutInitWindowPosition(100,100);

				glutCreateWindow("wesee - mt");

				glOrtho(0,displayImage->width(),0,displayImage->height(),-1,1);

				init();

				glutDisplayFunc(display);
				glutMouseFunc(mouse);
				glutMotionFunc(motion);
				glutKeyboardFunc(keyboard);


			}

			// ==============================================
			// auto grab cut
			// ==============================================
			Mat result;
			if(g_setting.enable_evaluation)
			{
				if(!img_ground_truth.data)
				{
					cerr<<" Can't open ground truth profile : "<<profile_name<<endl;
					return 0;
				}
				else
				{
					double cost = autoGrabCut(min, result);
					double score = evaluation(result);
					cout<<"file = "<<g_setting.matting_filename<<" score = "<<score<<" time = "<<cost<<"ms"<<endl;
				}
			}
			else
			{
				double cost = autoGrabCut(min, result);
				cout<<"file = "<<g_setting.matting_filename<<" time = "<<cost<<"ms"<<endl;
			}

			// ==============================================
			// save profile
			// ==============================================
			if(g_setting.output_profile)
			{
				string output_profile_name = profile_name.substr(0, profile_name.find_last_of(".")) + ".png";
				cv::imwrite(output_profile_name, result);
				cerr<<"output profile to "<<output_profile_name<<endl;
			}

			if(g_setting.enable_gui)
				glutMainLoop();		//note: this will NEVER return.
		}

	}

	return 0;
}
