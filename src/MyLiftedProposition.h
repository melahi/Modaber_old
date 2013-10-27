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

//	int id;

	bool initialValue;

	Literal *originalLiteral;

	vector <MyObject *> arguments;

	list <MyPartialAction *> adder;
	list <MyPartialAction *> deleter;
	list <MyPartialAction *> needer;


	MyLiftedProposition(): initialValue(false) {};
	MyLiftedProposition(const proposition *valProposition, vector <MyObject *> &arguments);

	//The following function find a MyLiftedProposition object from myProblem.liftedProposition which
	//is the same as the calling object (this). if there be no object, then at first the calling
	//object (this) insert it-self to the myProblem.liftedProposition and then return its pointer.
	MyLiftedProposition *find();

	void write (ostream &sout);

	virtual ~MyLiftedProposition();
};

} /* namespace mdbr */
#endif /* MYLIFTEDPROPOSITION_H_ */
