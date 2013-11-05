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
#include "MyAction.h"
#include "MyPartialAction.h"
#include "MyStateVariable.h"
#include "VALfiles/instantiation.h"



using namespace std;
using namespace Inst;


namespace mdbr{

class MyVariable;
class MyProposition;

class MyAction;

class MyPartialAction;

class MyStateVariable;
class MyStateValue;



class MyProposition {
public:

	int firstVisitedLayer;

	map <MyProposition *, int> lastLayerMutexivity;

	list <MyAction *> provider;

	bool checkMutex (int layerNumber, MyProposition *otherProposition);

	bool isMutex (int layerNumber, MyProposition *otherProposition);

	void insertMutex (int layerNumber, MyProposition *otherProposition);

	void visiting (int layerNumber, MyAction *action);


	Literal *originalLiteral;

	MyStateValue *stateValue;

	list <MyAction *> adderActions;
	list <MyAction *> deleterActions;
	list <MyAction *> userActions;    //The actions which need this proposition as their precondition



	vector <int> ids;  //id [i] means the id of proposition before execution of operator i

	bool initialValue;

	list <MyPartialAction *> adder;
	list <MyPartialAction *> deleter;

	MyProposition(Literal *originalLiteral):firstVisitedLayer(-1), originalLiteral(originalLiteral), stateValue(0), ids(current_analysis->the_domain->ops->size(), -2), initialValue(false){}
	MyProposition():firstVisitedLayer(-1), originalLiteral(0), stateValue(0), ids(current_analysis->the_domain->ops->size(), -2), initialValue(false) {}

	virtual void write (ostream &sout);

	virtual ~MyProposition(){}
};

class MyVariable{
public:

	vector <int> ids;

	PNE *originalPNE;

	list <MyAction *> modifierActions;
	list <MyAction *> userActions;


	list <MyPartialAction *> modifier;

	/* FIXME: for now we assume that every variable which appears in some precondition is
	 * important; but it is not completely true, perhaps a variable is appeared in an
	 * assignment and the assignee be an important one, then the variable is also should
	 * count as important variable!
	 */
	bool visitInPrecondition;

	double initialValue;

	MyVariable (PNE *originalPNE): ids(current_analysis->the_domain->ops->size(), -2), originalPNE(originalPNE), visitInPrecondition(false), initialValue(0){}
	MyVariable (): ids(current_analysis->the_domain->ops->size(), -2), originalPNE(0), visitInPrecondition(false), initialValue(0){}

	~MyVariable () { }

};


} /* namespace mdbr */

#endif /* MYATOM_H_ */
