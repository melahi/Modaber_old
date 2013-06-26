#ifndef MYTIMER_H_
#define MYTIMER_H_

#include <ctime>
using namespace std;

class MyTimer {

private:
	clock_t total;
	clock_t startingTime;
public:
	MyTimer();
	void startTimer();
	void endTimer();
	clock_t getTotalClock();
	double getDuration();
	virtual ~MyTimer();
};

#endif /* MYTIMER_H_ */
