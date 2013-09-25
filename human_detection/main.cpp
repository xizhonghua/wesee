#ifndef png_infopp_NULL
    #define png_infopp_NULL (png_infopp)NULL
#endif
#ifndef int_p_NULL
    #define int_p_NULL (int*)NULL
#endif

#include <dirent.h>
#include <sys/types.h>

#include <iostream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <set>
using namespace std;

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include "util.h"
#include <boost/bind.hpp>
#include <boost/timer.hpp>
#include <boost/format.hpp>
#include <boost/ref.hpp>

#include "poselet_api.h"

#include "config.h"
#include "util/Timer.h"



using namespace boost::gil;
using namespace poselet_api;



enum img_format
{
	JPEG,
	PNG,
	NOTKNOW
};

//#define debug

map<string, int> GT, RESULT;
double threshold;
string input_dir;
string modelname = "person";
bool output_score = false;
string output_name = "result.txt";
string score_name = "score.txt";
bool test_mode = false;
string test_filename;
set<string> file_to_test;

template <typename Value>
void append_hit(const Value& oh, vector<Value>& hits) {
  hits.push_back(oh);
}

template <typename View, typename Hit, typename Color>
void draw_hit(const View& v, const Hit& hit, const Color& c, int thickness=1) {
  typename View::value_type color;
  boost::gil::color_convert(c,color);
  int_bounds b(hit.x0,hit.y0,hit.width,hit.height);
  draw_bounds(v,color,b,thickness);

  render_text(v, b._min+int_point(thickness,thickness),
              str(boost::format("%4.2f") % hit.score), c, 1, 1);
}

bool file_exists(const char *filename)
{
  ifstream ifile(filename);
  return !!ifile;
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
	std::sort(files.begin(), files.end());
	return files;
}

vector<string> get_wrong_files(const string& output){
	vector<string> files;
	ifstream fs;
	fs.open(output.c_str(), ios::in);
	string str;
	while(fs>>str)
		files.push_back(str);
	fs.close();
	return files;
}

void readGroundTruth( const string& ground_truth)
{
	ifstream fs;
	string filename;
	int value;
	fs.open(ground_truth.c_str(), std::ifstream::in);
	while(fs>>filename>>value)
	{
		GT[filename] = value;
	}
}

void evaluation() {
	int sum = 0;
	int total = RESULT.size();
	for(map<string,int>::const_iterator it = RESULT.begin(); it != RESULT.end(); ++it)
	{
		if(GT[it->first] == it->second) sum++;
	}
	cout<<"============================================================"<<endl;
	cout<<"score:"<<(double)sum/total<<endl;
	cout<<"============================================================"<<endl;
}

int get_image_format(char* header)
{
	if ((unsigned int)(unsigned char)header[0]==0xFF && (unsigned int)(unsigned char)header[1]==0xD8)
		return JPEG;
	else if ((unsigned int)(unsigned char)header[0]==0x89 && (unsigned int)(unsigned char)header[1]==0x50 &&
			 (unsigned int)(unsigned char)header[2]==0x4E && (unsigned int)(unsigned char)header[3]==0x47 &&
			 (unsigned int)(unsigned char)header[4]==0x0D && (unsigned int)(unsigned char)header[5]==0x0A &&
			 (unsigned int)(unsigned char)header[6]==0x1A && (unsigned int)(unsigned char)header[7]==0x0A)
			return PNG;
		else
			return NOTKNOW;

}

void initial_config(){
	const config& c = config::get();
	c.PYRAMID_SCALE_RATIO = 1.5;
	threshold = 1.8;
}

bool parse_arg(int argc, char** argv){
	if(argc < 2) {
		return false;
	}
	int i=1;
	while(i<argc){
		string arg = argv[i];
		if(arg == "-t"){
			if(i+1 < argc && argv[i+1][0] != '-'){
				stringstream ss(argv[++i]);
				ss>>threshold;
			}
			else{
				return false;
			}
		}
		else if (arg == "-r"){
			if(i+1 < argc && argv[i+1][0] != '-'){
				stringstream rr(argv[++i]);
				rr>>config::get().PYRAMID_SCALE_RATIO;
			}else{
				return false;
			}
		}
		else if (arg == "-s"){
			output_score = true;
			score_name = argv[++i];
		}
		else if (arg == "-o"){
			if(i+1 < argc && argv[i+1][0] != '-'){
				output_name = argv[++i];
			}else{
				return false;
			}
		}
		else if(arg == "-if") {
			test_mode = true;
			test_filename = argv[++i];
		}
		else{
			input_dir = arg;
		}
		++i;
	}
	return true;
}

double human_detect(const string& img_path)
{
	try{
		  bgr8_image_t img;

		  // detect image type
		  ifstream ifs;
		  char memblock[10];
		  ifs.open(img_path.c_str(), ios::in|ios::binary);
		  ifs.read(memblock, 8);
		  ifs.close();
		  int img_type = get_image_format(memblock);
		  if (img_type==JPEG)
			  jpeg_read_image(img_path.c_str(),img);
		  else if (img_type==PNG)
			  png_read_image(img_path.c_str(),img);
		  else
			  return 0;

		  Image img_proxy(
			img.width(), img.height(), const_view(img).pixels().row_size(),
			Image::k8Bit, Image::kRGB, interleaved_view_get_raw_data(const_view(img)));

		  vector<ObjectHit> object_hits;
		  vector<PoseletHit> poselet_hits;

		  RunDetector(img_proxy, boost::bind(append_hit<PoseletHit>, _1,
			boost::ref(poselet_hits)), boost::bind(append_hit<ObjectHit>, _1,
			boost::ref(object_hits)), true, 0, false);

		  if (object_hits.size()>0)
		  {
			  return object_hits[0].score;
		  }
		  else
		  {
			  return 0;
		  }
	  }
	  catch (std::ios_base::failure& exception)
	  {
		  cout << "jpeg read exception" << endl;
		  return 0;
	  }
	  catch (int e)
	  {
		  cout << "unkown exception" << endl;
		  return 0;
	  }

	  return 0;
}

void load_test_files() {
	ifstream fs(test_filename.c_str());
	string test_file;
	while(fs>>test_file) {
		file_to_test.insert(test_file);
	}

	cout<<file_to_test.size()<<" files to test"<<endl;
}

int main( int argc, char* argv[] ) {
	initial_config();

	if (!parse_arg(argc, argv))
	{
		cout << "Usage: " << argv[0] << " input_dir"<<endl;
		return 0;
	}

	vector<string> files = get_files(input_dir);

	if(test_mode) load_test_files();

	InitDetector(".", modelname.c_str(), true);

	ofstream fs;
	fs.open(output_name.c_str(),std::ofstream::out);
	if(fs.bad())
	{
	  cout << "Failed to create result.txt" << endl;
	  return 0;
	}

	ofstream fscore;
	fscore.open(score_name.c_str(),std::ofstream::out);

	for (size_t i=0; i<files.size(); i++)
	{
		Timer t;
		if(test_mode && file_to_test.count(files[i])==0) {
			//cout<<"[TEST MODE] skip "<<files[i]<<endl;
			continue;
		}
		cerr << "Processing "<< files[i] << " ";
		double score = human_detect(input_dir + "/" + files[i]);
		int value = score > threshold ? 1 : 0;
		fs << files[i] << " " << value << endl;
		if (output_score)
		  fscore << files[i] << " " << score << endl;
		cerr << "label = " << value << " score = " << score << " "<< t.getElapsedMilliseconds() << "ms" <<endl;
	}
	fs.close();
	fscore.close();
	return 0;
}
