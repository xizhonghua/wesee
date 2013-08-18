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

bool parse_arg(int argc, char** argv);
void print_usage(int argc, char** argv);

vector<string> get_files(const string& input_dir);
void run_batch(const string& input_dir);


string g_filename;
string g_input_dir;
bool enable_gui = true;

#endif /* __MAIN_H__ */
