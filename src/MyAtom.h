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
#include "MyStateVariable.h"
#include "VALfiles/instantiation.h"



using namespace std;
using namespace Inst;


namespace mdbr{

class MyVariable;
class MyAtom;
class   MyProposition;
class   MyValue;

class MyAction;
class   MyGroundedAction;

class MyStateVariable;
class MyStateValue;

class MyAssignment;

class MyAtom {
public:

	int firstVisitedLayer;

	map <MyAtom *, int> lastLayerMutexivity;

	list <MyGroundedAction *> provider;

	bool checkMutex (int layerNumber, MyAtom *otherAtom);

	bool isMutex (int layerNumber, MyAtom *otherAtom);

	void insertMutex (int layerNumber, MyAtom *mutexAtom);

	void visiting (int layerNumber, MyGroundedAction*action);

	MyAtom();

	virtual void write (ostream &sout) = 0;

	virtual ~MyAtom();
};

class MyProposition: public MyAtom {
public:

	Literal *originalLiteral;

	MyStateValue *stateValue;

	list <MyAction *> adderActions;
	list <MyAction *> deleterActions;
	list <MyAction *> userActions;    //The actions which need this proposition as their precondition


	MyProposition(Literal *originalLiteral):MyAtom(), originalLiteral(originalLiteral), stateValue(0){}
	MyProposition():MyAtom(), originalLiteral(0), stateValue(0) {}

	virtual void write (ostream &sout);

	virtual ~MyProposition(){}
};

class MyValue: public MyAtom {
public:
	MyVariable *variable;
	double value;

//	int globalValueId;

	vector <int> ids;

	MyValue(MyVariable *variable, double value): MyAtom(), variable(variable), value(value){}
	MyValue(): MyAtom(){}

	bool operator < (const MyValue &otherValue) const;

	virtual void write (ostream &sout);

	~MyValue(){}

};

class MyVariable{
private:

	map <double, MyValue>::iterator interanalState;

public:
	PNE *originalPNE;

	list <MyAction *> modifierActions;
	list <MyAction *> userActions;

	list <MyAssignment *> assigner;


	/* FIXME: for now we assume that every variable which appears in so me precondition is
	 * important; but it is not completely true, perhaps a variable is appeared in an
	 * assignment and the assignee be an important one, then the variable is also should
	 * count as important variable!
	 */
	bool visitInPrecondition;

	MyValue *initialValue;

	map <double, MyValue> domain;

	MyVariable (PNE *originalPNE): originalPNE(originalPNE), visitInPrecondition(false), initialValue(NULL){}
	MyVariable (): originalPNE(0), visitInPrecondition(false), initialValue(NULL){}


	void valueIsFound (double value, int layerNumber, MyGroundedAction *action){
		if (domain.find(value) == domain.end()){
			domain[value] = MyValue (this, value);
		}
		domain[value].visiting(layerNumber, action);
	}

	void restart () {
		interanalState = domain.begin();
	}

	bool isEnd () {
		if (interanalState == domain.end()){
			return true;
		}
		return false;
	}

	void next (int layerNumber){
		while (interanalState != domain.end() && (++interanalState)->second.firstVisitedLayer > layerNumber){
			;
		}
	}

	MyValue *getValue(){
		return &(interanalState->second);
	}

	void write (ostream &sout) const{
		sout << "[";
		originalPNE->write(sout);

		map <double, MyValue>::const_iterator it, itEnd;
		it = domain.begin();
		itEnd = domain.end();

		for (; it != itEnd; ++it){
			sout << ", " << it->second.value;
		}
		sout <<"]";
	}

	~MyVariable () { }

};


} /* namespace mdbr */

#endif /* MYATOM_H_ */