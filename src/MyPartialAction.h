/*
 * MyPartialAction.h
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#ifndef MYPARTIALACTION_H_
#define MYPARTIALACTION_H_

#include "MyLiftedProposition.h"
#include "MyOperator.h"

#include <vector>


using namespace std;


namespace mdbr {


class MyLiftedProposition;

class MyLiftedPartialAction{
private:
	bool grounded;
	vector <MyObject *> selectedObject;  //A temporary vector used in grounding
	vector <int> IdOfSelectedObject;  //A temporary vector used in grounding

public:

	const proposition *originalPredicate;

	int nArguments;
	vector <MyType *> argument;

	vector <int> placement;   //placement[i] determine the place i'th object in operator argument. (e.g.    move (x, y, z): pre(...) && add(on(x, z), ...) && del(...)   ==> for predicate "add(on(x, z))": placement[1] = 3

	propositionKind pKind;
	MyOperator *op;

	MyLiftedPartialAction (): grounded(false) {}
	void prepare (MyOperator *op, propositionKind pKind, const proposition *prop);
	void grounding();
	void grounding (unsigned int argumentIndex);

};



class MyPartialAction {
public:

	int id;

	MyLiftedProposition *proposition;

	MyLiftedPartialAction *liftedPartialAction;

	MyOperator *op;

	propositionKind pKind;

	vector <int> objectId;

//	MyPartialAction(propositionKind pKind, MyOperator *op, MyLiftedPartialAction *predicate, vector <MyObject*> &argument, vector <int> &objectId, int id);
	MyPartialAction() {}

	void prepare (propositionKind pKind, MyOperator *op, MyLiftedPartialAction *predicate, vector <MyObject*> &argument, vector <int> &objectId, int id);

	bool isForSameAction (MyPartialAction *other);

	void write (ostream &sout, bool isEndl = true);

	virtual ~MyPartialAction();
};

} /* namespace mdbr */
#endif /* MYPARTIALACTION_H_ */
