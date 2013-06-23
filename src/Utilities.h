#ifndef UTILITIES_H_
#define UTILITIES_H_


#include <vector>
#include <cmath>


using namespace std;


#define CANT_HANDLE(x) cerr << "********************************" << x << "********************************" << endl;
#define EPSILON 1e-9


/*
 * selectRandom function select "numberOfSelected" unique number from the range of ["from", "to")
 * if "numberOfSelected" be greater than ("to" - "from") then we decrease "numberOfSelected" to
 * the value of ("to" - "from").
 */
vector <int> selectRandom (int from, int to, int numberOfSelected);


//This function is just a test for selectRandom function.
int testSelectRandom();



#endif /* UTILITIES_H_ */
