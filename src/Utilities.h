#ifndef UTILITIES_H_
#define UTILITIES_H_


#include <vector>
#include <cmath>
#include "MyTimer.h"


using namespace std;


#define CANT_HANDLE(x) cerr << "********************************" << x << "********************************" << endl;
#define EPSILON 1e-9


#define isVisited(firstVisitedLayer, layerNumber) (firstVisitedLayer != -1 && firstVisitedLayer <= layerNumber)



/*
 * selectRandom function select "numberOfSelected" unique number from the range of ["from", "to")
 * if "numberOfSelected" be greater than ("to" - "from") then we decrease "numberOfSelected" to
 * the value of ("to" - "from").
 */
inline vector <int> selectRandomly (int from, int to, int numberOfSelected);

/*
 * select a number from 0 to "probabilityDistribution.size() - 1" based
 * on giver distribution
 */
inline int selectRandomly (const vector <double> &probabilityDistribution);


/*
 * In the following function myVector will be normalize so that every elements of it should be in
 * the range of [0, 1] and sum of the myVector's element should be 1
 */
inline void normolizing (vector <double> &myVector);
inline void normolizing (vector <double> &myVector, double sumOfMyVector);


//calculate sum of myVector's elements
inline double vectorSum (const vector <double> &myVector);


//This function is just a test for selectRandom function.
int testSelectRandom();



#endif /* UTILITIES_H_ */
