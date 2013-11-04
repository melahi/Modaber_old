/*
 * MyLiftedProposition.h
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#ifndef MYLIFTEDPROPOSITION_H_
#define MYLIFTEDPROPOSITION_H_

#include "MyObject.h"
#include "MyPartialAction.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include "MyOperator.h"


#include <iostream>
#include <vector>
#include <list>

using namespace std;
using namespace VAL;
using namespace Inst;


namespace mdbr {

class MyPartialAction;


class MyLiftedProposition {
public:

	vector <int> ids;  //id [i] means the id of proposition before execution of operator i

	bool initialValue;

	list <MyPartialAction *> adder;
	list <MyPartialAction *> deleter;
//	list <MyPartialAction *> needer;


	MyLiftedProposition(): ids(myProblem.operators.size(), -2), initialValue(false) {};

	void write (ostream &sout);

	virtual ~MyLiftedProposition();
};

} /* namespace mdbr */
#endif /* MYLIFTEDPROPOSITION_H_ */
