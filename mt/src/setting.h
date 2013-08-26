#ifndef __SETTING_H__
#define __SETTING_H__

#include <string>
using namespace std;

class setting {
public:
	string matting_filename;
	string training_filename;
	string input_dir;
	string training_dir;
	bool enable_gui = true;
	bool enable_evaluation = false;

	bool training_mode = false;
	bool training_batch_mode = false;
	bool matting_batch_mode = false;

	bool matting_mode = false;
	int max_refine_iterations = 5;

	bool output_profile = false;

	bool evaluation_mode = false;
	string profile_filename;
	string profile_ground_truth_filename;

	bool resize_mode = false;
	string resize_filename;
	int resize_long_edge;
};

#endif /*  __SETTING_H__ */
