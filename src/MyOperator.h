/*
 * MyOperator.h
 *
 *  Created on: Sep 22, 2013
 *      Author: sadra
 */

#ifndef MYOPERATOR_H_
#define MYOPERATOR_H_

#include "VALfiles/parsing/ptree.h"
#include "MyObject.h"
#include "MyAssignment.h"
#include "MyComparison.h"

#include <vector>

using namespace VAL;
using namespace std;

namespace mdbr {

enum propositionKind {precondition, addEffect, deleteEffect};

class MyLiftedPartialAction;
class MyLiftedAssignment;
class MyLiftedComparison;

class MyOperator {
private:
	bool grounded;
public:

	int id;

	vector <MyType *> argument;

	vector <MyLiftedPartialAction *> myAddEffect;
	vector <MyLiftedPartialAction *> myDeleteEffect;
	vector <MyLiftedPartialAction *> myPrecondition;
	vector <MyLiftedComparison *> myComparison;
	vector <MyLiftedAssignment *> myAssignment;

	operator_ *originalOperator;
	vector <int> offset;      							//The offset used for unifications


	MyOperator();

	void prepare (operator_ *originalOperator, int id);

	void prepareSimpleEffect (pc_list <simple_effect *> &valEffectList, propositionKind pKind, vector <MyLiftedPartialAction *> &effectList);
	void prepareAssignment (pc_list <assignment *> &assignmentList);
	void preparePreconditions (goal *gl);


	virtual ~MyOperator();
};

} /* namespace mdbr */
#endif /* MYOPERATOR_H_ */
