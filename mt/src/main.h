#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include <opencv2/highgui/highgui.hpp>

#include "Image.h"
#include "Matting.h"
#include "Timer.h"
#include "setting.h"

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
