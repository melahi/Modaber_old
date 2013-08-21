/*
 * MyEnvironment.h
 *
 *  Created on: Aug 20, 2013
 *      Author: sadra
 */

#ifndef MYENVIRONMENT_H_
#define MYENVIRONMENT_H_

#include <vector>
using namespace std;




namespace mdbr {

class MyEnvironment {
private:
	void prepareProbabilityMatrixOfStateVariable (int layerNumber, int variableId);
	void prepareLayer (int layerNumber);
public:

	/*
	 * The probability[i][j][k][l] specify the probability of transition of variable j from k in layer i + 1 to value l in layer i
	 * The reason for such strange convention is that we want to search in backward manner
	 */
	vector < vector < vector < vector <double> > > > probability;

	int length;

	MyEnvironment(): length(0){};

	void prepare (int nSignificantTimePoint);

	virtual ~MyEnvironment();
};

} /* namespace mdbr */
#endif /* MYENVIRONMENT_H_ */
