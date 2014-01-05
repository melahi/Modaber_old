/*
 * PlanningGraphProposition.h
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#ifndef MYATOM_H_
#define MYATOM_H_


#include <list>
#include <map>
#include <set>
#include <iostream>
#include "MyPartialAction.h"
#include "VALfiles/instantiation.h"



using namespace std;
using namespace Inst;


namespace mdbr{

class MyVariable;
class MyProposition;


class MyPartialAction;




class MyProposition {
public:

	Literal *originalLiteral;

	vector <int> ids;  //id [i] means the id of proposition before execution of operator i

	list <MyPartialAction *> adder;
	list <MyPartialAction *> deleter;

	MyProposition(Literal *originalLiteral):originalLiteral(originalLiteral), ids(current_analysis->the_domain->ops->size(), -2){}
	MyProposition():originalLiteral(0), ids(current_analysis->the_domain->ops->size(), -2) {}

	virtual void write (ostream &sout);

	virtual ~MyProposition(){}
};

class MyVariable{
public:

	vector <int> ids;

	PNE *originalPNE;


	list <MyPartialAction *> modifier;

	/* FIXME: for now we assume that every variable which appears in some precondition is
	 * important; but it is not completely true, perhaps a variable is appeared in an
	 * assignment and the assignee be an important one, then the variable is also should
	 * count as important variable!
	 */

	MyVariable (PNE *originalPNE): ids(current_analysis->the_domain->ops->size(), -2), originalPNE(originalPNE){}
	MyVariable (): ids(current_analysis->the_domain->ops->size(), -2), originalPNE(0) {}

	~MyVariable () { }

};


} /* namespace mdbr */

#endif /* MYATOM_H_ */
