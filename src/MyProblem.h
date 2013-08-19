/*
 * MyProblem.h
 *
 *  Created on: Aug 6, 2013
 *      Author: sadra
 */

#ifndef MYPROBLEM_H_
#define MYPROBLEM_H_

#include "MyAction.h"
#include "MyAtom.h"
#include "MyStateVariable.h"

#include <vector>


using namespace std;

namespace mdbr {

class MyProblem {
private:
	void readingSASPlusFile();
	void buildingDTG();
public:

	vector <MyAction> actions;
	vector <MyProposition> propositions;
	vector <MyVariable> variables;
	vector <MyStateVariable> stateVariables;

	vector <double> initialValue;  //The initial value for each variable


	void updateInitialValues ();

	void initializing();

	void print();

	MyProblem();
	virtual ~MyProblem();
};

extern MyProblem myProblem;

} /* namespace mdbr */
#endif /* MYPROBLEM_H_ */
