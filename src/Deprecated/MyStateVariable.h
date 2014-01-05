/*
 * MyStateVariable.h
 *
 *  Created on: Aug 17, 2013
 *      Author: sadra
 */

#ifndef MYSTATEVARIABLE_H_
#define MYSTATEVARIABLE_H_

#include <vector>
#include "MyAtom.h"
#include "MyAction.h"

using namespace std;

namespace mdbr {

class MyStateVariable;
class MyStateValue;
class MyProposition;
class MyAction;

class MyStateVariable {
public:

	int variableId;

	vector <MyStateValue> domain;

	MyStateVariable();

	void write (ostream &sout);

	virtual ~MyStateVariable();

};

class MyStateValue{
public:

	int valueId;

	int firstVisitedLayer;


	MyProposition *theProposition;
	MyStateVariable *theStateVariable;
	vector < list<MyAction*> > providers; // providers[i] determines actions which transform theStateVariable from i to valueId;

	MyStateValue();
	void initialize(int valueId, MyProposition *theProposition, MyStateVariable *theStateVariable);

	void write (ostream &sout);

};

} /* namespace mdbr */
#endif /* MYSTATEVARIABLE_H_ */
