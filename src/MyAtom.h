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

//	vector <int> ids;

	MyValue(MyVariable *variable, double value): MyAtom(), variable(variable), value(value){}
	MyValue(): MyAtom(){}

	bool operator < (const MyValue &otherValue) const;

	virtual void write (ostream &sout);

	~MyValue(){}

};

class MyRange {
public:
	double starting, ending;

	vector <int> ids;

	MyRange(): starting(0), ending(0), ids (current_analysis->the_domain->ops->size(), -2) {}

	bool operator < (const MyRange &a) const{
		if (starting != a.starting) return starting < a.starting;
		return ending < a.ending;
	}
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

	set <MyRange> domainRange;


	MyVariable (PNE *originalPNE): originalPNE(originalPNE), visitInPrecondition(false), initialValue(NULL){}
	MyVariable (): originalPNE(0), visitInPrecondition(false), initialValue(NULL){}


	void valueIsFound (double value, int layerNumber, MyGroundedAction *action){
		if (domain.find(value) == domain.end()){
			domain[value] = MyValue (this, value);
		}
		domain[value].visiting(layerNumber, action);
	}

	void completeDomainRange() {
		if (domain.size() < 1 || originalPNE->getStateID() == -1){
			return;
		}
		map <double, MyValue>::iterator it, itEnd, it2;
		it = domain.begin();
		itEnd = domain.end();

		domain[NegInf].value = NegInf;
		domain[PosInf].value = PosInf;

		for (; it != itEnd; ++it){
			for (it2 = it; it2 != itEnd; ++it2){
				MyRange a;
				a.starting = it->first;
				a.ending = it2->first;
				domainRange.insert(a);
			}
		}
	}


	double findGreatestMinimum (double a){
		map <double, MyValue>::iterator it, itEnd;
		it = domain.begin();
		itEnd = domain.end();
		double ret = it->first;
		it++;
		for (; it != itEnd; ++it){
			if (it->first > a){
				return ret;
			ret = it->first;
			}
		}
		return ret;
	}

	double findLeastMaximum (double a){
		map <double, MyValue>::reverse_iterator it, itEnd;
		it = domain.rbegin();
		itEnd = domain.rend();
		double ret = it->first;
		it++;
		for (; it != itEnd; ++it){
			if (it->first < a){
				return ret;
			ret = it->first;
			}
		}
		return ret;
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
