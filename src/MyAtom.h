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
#include "VALfiles/instantiation.h"


using namespace std;
using namespace Inst;


namespace mdbr{

class MyAction;

class MyAtom {
public:

	int firstVisitedLayer;

	map <MyAtom *, int> lastLayerMutex;

	list <MyGroundedAction*> provider;

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

	list <MyAction *> adderActions;
	list <MyAction *> deleterActions;
	list <MyAction *> userActions;    //The actions which need this proposition as their precondition


	MyProposition(Literal *originalLiteral):MyAtom(), originalLiteral(originalLiteral){}

	virtual void write (ostream &sout){
		originalLiteral->write(sout);
	}

	virtual ~MyProposition(){}
};

class MyValue: public MyAtom {
public:
	MyVariable *variable;
	double value;

	MyValue(MyVariable *variable, double value): variable(variable), value(value){}
	MyValue(){}

	bool operator < (const MyValue &otherValue) const{
		if (variable->originalPNE->getStateID() == otherValue.variable->originalPNE->getStateID()){
			return value < otherValue.value;
		}
		return variable->originalPNE->getStateID() < otherValue.variable->originalPNE->getStateID();
	}

	virtual void write (ostream &sout){
		sout << "(";
		variable->originalPNE->write(sout);
		sout << ": " << value << ")";
	}

	~MyValue(){}

};

class MyVariable{
private:

	list <MyValue>::iterator interanalState;

public:
	PNE *originalPNE;

	list <MyAction *> modifierActions;
	list <MyAction *> userActions;


	set <MyValue> domain;

	MyVariable (PNE *originalPNE): originalPNE(originalPNE){}

	void findValue (double value, int layerNumber, MyGroundedAction *action){
		pair <set <MyValue>::iterator, bool > ret;
		ret = domain.insert(MyValue (this, value));
		MyValue *myValue= ret.first;
		myValue->visiting(layerNumber, action);
	}

	void restart (){
		interanalState = domain.begin();
	}
	bool isEnd (){
		if (interanalState == domain.end()){
			return true;
		}
		return false;
	}
	void next (){
		interanalState++;
	}
	MyValue *getValue(){
		return *interanalState;
	}
};


} /* namespace mdbr */

#endif /* MYATOM_H_ */
