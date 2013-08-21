
#include "Utilities.h"
#include <cstdlib>
#include <vector>
#include <iostream>


using namespace std;

/*
 * This function select "numberOfSelected" unique numbers from the range of ["from", "to")
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

inline int selectRandomly (const vector <double> &probabilityDistribution){
	double myRand = drand48();
	int j = 0;
	while (myRand - probabilityDistribution[j] > EPSILON){
		myRand -= probabilityDistribution[j];
		j++;
	}
	return j;
}


inline void normolizing (vector <double> &myVector){
	normolizing(myVector, vectorSum(myVector));
}

inline void normolizing (vector <double> &myVector, double sumOfMyVector){
	int lng = myVector.size();
	for (int i = 0; i < lng; i++){
		myVector[i] /= sumOfMyVector;
	}
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

int testSelectRandom(){
	for (int i = 0; i < 1000; i++){
		vector <int> myRandom = selectRandomly(2, 12, 5);
		vector <bool> marked (10, false);
		for (unsigned int j = 0; j < myRandom.size(); j++){
			if (myRandom[j] < 2 || myRandom[j] >= 12){
				cout << "Error: out of range!" << endl;
			}
			if (marked[myRandom[j] - 2]){
				cout << "One number appeared more than one time!" << endl;
			}
			marked[myRandom[j] - 2] =  true;
		}
	}
	cout << "Finished" << endl;
	return 0;
}




