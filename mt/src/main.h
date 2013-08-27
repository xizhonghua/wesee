#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;

#include "setting.h"
#include "Matting.h"
#include "Statistics.h"


#include "Global.h"
#include "GrabCut.h"

#include "Timer.h"
#include "MatHelper.h"

#define LONG_EDGE_PX 360


bool parse_arg(int argc, char** argv);
void print_usage(int argc, char** argv);

string get_profile_name(const string& input);

vector<string> get_files(const string& input_dir);

void evaluate(const string& profile, const string& ground_truth);
bool matting(const string& input, const string& output, Mat* min, Mat* out, const string* ground_truth, double* score);
bool training(Statistics& stat, const string& input, const string& profile);
double autoGrabCut(GrabCut* gc, const Mat& ori, const Mat& min, const Mat& trimap, Mat& output);
void run_batch(const string& input_dir);
void train_batch(const string& input_dir);


setting g_setting;

#endif /* __MAIN_H__ */
