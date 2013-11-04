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

#include <list>
#include <vector>

using namespace VAL;
using namespace std;

namespace mdbr {

class MyPartialOperator;
class MyLiftedAssignment;
class MyLiftedComparison;

class MyOperator {
private:
	bool grounded;

	list <MyPartialOperator *>::iterator findPartialOperator (const MyPartialOperator *a);
public:

	operator_ *originalOperator;
	int id;

	vector <MyType *> argument;
	vector <int> offset;      							//The offset used for unifications

	list <MyPartialOperator *> partialOperator;


	MyOperator();

	void prepare (operator_ *originalOperator, int id);

	void prepareSimpleEffect (pc_list <simple_effect *> &valEffectList, bool addEffect);
	void prepareAssignment (pc_list <assignment *> &assignmentList);
	void preparePreconditions (goal *gl);


	virtual ~MyOperator();
};

} /* namespace mdbr */
#endif /* MYOPERATOR_H_ */
