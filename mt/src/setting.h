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
	bool matting_mode = false;
	bool training_mode = false;
	bool training_batch_mode = false;
	bool matting_batch_mode = false;
};

#endif /*  __SETTING_H__ */
