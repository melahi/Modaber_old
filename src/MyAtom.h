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


	MyProposition(Literal *originalLiteral):MyAtom(), originalLiteral(originalLiteral){}
	MyProposition():MyAtom() {}

	virtual void write (ostream &sout){
		originalLiteral->write(sout);
	}

	virtual ~MyProposition(){}
};

class MyValue: public MyAtom {
public:
	MyVariable *variable;
	double value;

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


	map <double, MyValue> domain;

	MyVariable (PNE *originalPNE): originalPNE(originalPNE){}
	MyVariable (){}


	void findValue (double value, int layerNumber, MyGroundedAction *action){
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
