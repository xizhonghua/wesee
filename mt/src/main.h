#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

#include <opencv2/highgui/highgui.hpp>

#include "Matting.h"

#include "setting.h"
#include "Global.h"
#include "GrabCut.h"

#include "Timer.h"
#include "MatHelper.h"

#define LONG_EDGE_PX 360


bool parse_arg(int argc, char** argv);
void print_usage(int argc, char** argv);

string get_profile_name(const string& input);

vector<string> get_files(const string& input_dir);

bool matting(const string& input, const string& output, Mat* min, Mat* out, const string* ground_truth, double* score);
bool training(const string& input, const string& profile);
void run_batch(const string& input_dir);
void train_batch(const string& input_dir);


setting g_setting;

#endif /* __MAIN_H__ */
