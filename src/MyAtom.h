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
#include "VALfiles/instantiation.h"



using namespace std;
using namespace Inst;


namespace mdbr{

class MyVariable;
class MyProposition;

class MyAction;

class MyPartialAction;



class MyProposition {
public:

	int firstVisitedLayer;

	bool possibleEffective;

	map <MyProposition *, int> lastLayerMutexivity;

	list <MyAction *> provider;  //In graph plan (provider of this proposition in graphpaln)

	bool checkMutex (int layerNumber, MyProposition *otherProposition);

	bool isMutex (int layerNumber, MyProposition *otherProposition);

	void insertMutex (int layerNumber, MyProposition *otherProposition);

	void visiting (int layerNumber, MyAction *action);


	Literal *originalLiteral;

	vector <int> ids;  //id [i] means the id of proposition before execution of operator i


	list <MyPartialAction *> adder;
	list <MyPartialAction *> deleter;

	vector <MyAction *> adder_groundAction;
	vector <MyAction *> deleter_groundAction;

	MyProposition(Literal *originalLiteral):firstVisitedLayer(-1), possibleEffective(false), originalLiteral(originalLiteral), ids(current_analysis->the_domain->ops->size(), -2) {}
	MyProposition():firstVisitedLayer(-1), possibleEffective (false), originalLiteral(0), ids(current_analysis->the_domain->ops->size(), -2) {}

	virtual void write (ostream &sout);

	virtual ~MyProposition(){}
};

class MyVariable{
public:

	vector <int> ids;

	PNE *originalPNE;

	map <double, vector <int> > domain;  //The first element of the map is a member of domain and second one is ids associated with the element


	list <MyPartialAction *> modifier;
	vector <MyAction *> modifier_groundAction;

	MyVariable (PNE *originalPNE): ids(current_analysis->the_domain->ops->size(), -2), originalPNE(originalPNE){}
	MyVariable (): ids(current_analysis->the_domain->ops->size(), -2), originalPNE(0){}

	~MyVariable () { }

};

enum boundKind { upperBound, lowerBound };
class MyBound {
public:
	boundKind kind;
	map <double, vector <int> >::iterator member;
};


} /* namespace mdbr */

#endif /* MYATOM_H_ */
