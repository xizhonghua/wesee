/*
 * Timer.h
 *
 *  Created on: Aug 6, 2013
 *      Author: zhonghua
 */


#ifndef G_UTILS_TIMER_H_
#define G_UTILS_TIMER_H_

#include <sys/time.h>

class Timer {
public:
	Timer(){
		this->start();
	}
	virtual ~Timer(){};

	void start() {
		gettimeofday(&m_start_time, NULL);
	}

	void restart() {
		this->start();
	}

	double getElapsedMilliseconds(){
		timeval now;
		gettimeofday(&now, NULL);
		double milliseconds = ((now.tv_sec - m_start_time.tv_sec)*1000000.0 + (now.tv_usec - m_start_time.tv_usec))/1000.0;
		return milliseconds;
	}
private:
	timeval m_start_time;
};

#endif /* G_UTILS_TIMER_H_ */
