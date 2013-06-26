/*
 * myTimer.cpp
 *
 *  Created on: Jun 26, 2013
 *      Author: sadra
 */

#include "MyTimer.h"
#include <ctime>
using namespace std;

MyTimer::MyTimer() {
	total = 0;
	startingTime = -1;
}

void MyTimer::startTimer() {
	startingTime = clock();
}

void MyTimer::endTimer() {
	if (startingTime != -1){
		total += (clock() - startingTime);
	}
	startingTime = -1;
}

clock_t MyTimer::getTotalClock(){
	return total;
}

double MyTimer::getDuration(){
	return ((double) total / (double) CLOCKS_PER_SEC);
}

MyTimer::~MyTimer() {
	// TODO Auto-generated destructor stub
}

