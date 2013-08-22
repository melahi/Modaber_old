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
#include "MyEnvironment.h"

#include <vector>


using namespace std;

namespace mdbr {

class MyProblem {
private:
	void readingSASPlusFile();
	void buildingDTG();


	void filterVariables();

public:

	bool usingSASPlus;

	vector <MyAction> actions;
	vector <MyProposition> propositions;
	vector <MyVariable> variables;
	vector <MyStateVariable> stateVariables;

	MyEnvironment environment;

	vector <double> initialValue;  //The initial value for each variable


	void updateInitialValues ();

	void initializing(bool usingSASPlus);

	void print();

	MyProblem();
	virtual ~MyProblem();
};

extern MyProblem myProblem;

} /* namespace mdbr */
#endif /* MYPROBLEM_H_ */
