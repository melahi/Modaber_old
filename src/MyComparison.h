/*
 * MyRelation.h
 *
 *  Created on: Oct 10, 2013
 *      Author: sadra
 */

#ifndef MYCOMPARISON_H_
#define MYCOMPARISON_H_

#include "MyObject.h"
#include "VALfiles/parsing/ptree.h"
#include "MyOperator.h"
#include "MyAtom.h"

#include <map>
#include <string>


using namespace std;

namespace mdbr {

class MyOperator;

class MyLiftedComparison{
private:
	bool grounded;
	map <string, MyObject *> selectedObject;
	map <string, int> objectId;

public:
	map <string, MyType *>  types;
	map <string, int> placement;
	const comparison *originalComparison;

	MyOperator *op;

	void prepare (MyOperator *op_,const comparison *originalComparison_);

	void findTypes (const expression *exp);

	void grounding();

	void grounding (map <string, MyType *>::iterator it);

};

class MyComparison {

private:
	list <MyVariable *> myCreatedVariables;

	map <const func_term *, MyValue *> selectedValues;

public:
	map <const func_term *, MyVariable *> variables;
	list < pair < list < MyValue* >, bool> > possibleValues;


	map <string, MyObject *> selectedObject;
	map <string, int> objectId;

	MyOperator *op;
	MyLiftedComparison *liftedComparison;

	int comparisonId;



	MyComparison();
	void prepare (MyOperator *op_, MyLiftedComparison *liftedComparison_, map <string, MyObject *> &selectedObject_, map <string, int> &objectId_, int comparisonId_);

	void findVariables (const expression *exp);

	void findPossibleValues ();
	void findPossibleValues ( map <const func_term *, MyVariable *>::iterator it);

	bool evalute ();

	double evalute (const expression *exp);

	void write (ostream &sout);

	virtual ~MyComparison(){
		list <MyVariable *>::iterator it, itEnd;
		it = myCreatedVariables.begin();
		itEnd = myCreatedVariables.end();
		for (; it != itEnd; ++itEnd){
			delete(*it);
		}
	}
};

} /* namespace mdbr */
#endif /* MYCOMPARISON_H_ */
