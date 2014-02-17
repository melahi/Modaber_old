#ifndef UTILITIES_H_
#define UTILITIES_H_


#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "MyTimer.h"


using namespace std;


#define CANT_HANDLE(x) cerr << "********************************" << x << "********************************" << endl;
#define EPSILON 1e-9


#define isVisited(firstVisitedLayer, layerNumber) (firstVisitedLayer != -1 && firstVisitedLayer <= layerNumber)
#define isVisited2(firstVisitedLayer) (firstVisitedLayer != -1)

#define initializeIterator(begining, ending, container) begining = container.begin(); ending = container.end();
#define FOR_ITERATION(begining, ending, container) \
		initializeIterator(begining, ending, container) \
		for (;begining != ending; ++begining)

extern const double undefinedValue;
extern const double infinite;


/*
 * selectRandom function select "numberOfSelected" unique number from the range of ["from", "to")
 * if "numberOfSelected" be greater than ("to" - "from") then we decrease "numberOfSelected" to
 * the value of ("to" - "from").
 */
inline vector <int> selectRandomly (int from, int to, int numberOfSelected){
	vector <int> ret;
	vector <bool> markedNumber(to - from, false);
	numberOfSelected = min (numberOfSelected, (to - from));

	for (int i = 0; i < numberOfSelected; i++){
		int selectedNumber = (drand48() * (to - from - i));
		int k = 0;
		int j = 0;
		while (true){
			if (!markedNumber[k]){
				if (j == selectedNumber){
					break;
				}
				j++;
			}
			k++;
			if (k == (to - from)){
				k = 0;
			}
		}
		markedNumber[k] = true;
		ret.push_back(k + from);
	}
	return ret;
}

/*
 * select a number from 0 to "probabilityDistribution.size() - 1" based
 * on giver distribution
 */
inline int selectRandomly (const vector <double> &probabilityDistribution){
	double myRand = drand48();
	int j = 0;
	while (myRand - probabilityDistribution[j] > EPSILON){
		myRand -= probabilityDistribution[j];
		j++;
	}
	return j;
}


//calculate sum of myVector's elements
inline double vectorSum (const vector <double> &myVector){
	int lng = myVector.size();
	double sum = 0;
	for (int i = 0; i < lng; i++){
		sum += myVector[i];
	}
	return sum;
}


/*
 * In the following function myVector will be normalize so that every elements of it should be in
 * the range of [0, 1] and sum of the myVector's element should be 1
 */
inline void normolizing (vector <double> &myVector, double sumOfMyVector){
	int lng = myVector.size();
	for (int i = 0; i < lng; i++){
		myVector[i] /= sumOfMyVector;
	}
}
inline void normolizing (vector <double> &myVector){
	normolizing(myVector, vectorSum(myVector));
}

//This function is just a test for selectRandom function.
int testSelectRandom();



#endif /* UTILITIES_H_ */
