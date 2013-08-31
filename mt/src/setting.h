#ifndef __SETTING_H__
#define __SETTING_H__

#include <string>
using namespace std;

class setting {
public:
	setting(){
		this->enable_gui = true;
		this->enable_evaluation = false;
		this->training_mode = false;
		this->training_batch_mode = false;
		max_training_images = 0;

		this->matting_batch_mode = false;

		this->matting_mode = false;
		this->max_refine_iterations = 3;

		this->output_prediction = false;
		this->output_profile = true;

		this->resize_mode = false;
		this->evaluation_mode = false;

		this->training_database_filename = "train.bin";
	}
	string matting_filename;

	string input_dir;
	string training_dir;
	int max_training_images;

	bool enable_gui;
	bool enable_evaluation;

	bool training_mode;
	bool training_batch_mode;
	string training_filename;
	string training_database_filename;

	bool matting_batch_mode;

	bool matting_mode;
	int max_refine_iterations;

	bool output_profile;
	bool output_prediction;

	bool evaluation_mode;
	string profile_filename;
	string profile_ground_truth_filename;

	bool resize_mode;
	string resize_filename;
	int resize_long_edge;
};

#endif /*  __SETTING_H__ */
