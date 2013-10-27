
#include "Utilities.h"
#include <vector>
#include <iostream>


using namespace std;

const double undefinedValue = numeric_limits <double>::min();



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




