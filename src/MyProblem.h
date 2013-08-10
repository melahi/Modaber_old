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

#include <vector>


using namespace std;

namespace mdbr {

class MyProblem {
public:

	vector <MyAction> actions;
	vector <MyProposition> propositions;
	vector <MyVariable> variables;

	vector <double> initialValue;  //The initial value for each variable


	void updateInitialValues ();

	MyProblem();
	virtual ~MyProblem();
};

MyProblem myProblem;

} /* namespace mdbr */
#endif /* MYPROBLEM_H_ */
