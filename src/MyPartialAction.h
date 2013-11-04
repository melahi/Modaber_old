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
#include "VALfiles/parsing/ptree.h"

#include <vector>
#include <string>
using namespace std;



namespace mdbr {

class MyLiftedProposition;

class MyPartialOperator{
private:
	map <string, MyObject*> selectedObject;  //A temporary vector used in grounding
	map <string, int> IdOfUnification;  //A temporary vector used in grounding

	void findTypes(const VAL::expression *exp);

public:

	MyOperator *op;
	map <string, MyType *> argument;
	map <string, int> placement;



	list <const proposition *> precondition;
	list <const proposition *> addEffect;
	list <const proposition *> deleteEffect;
	list <const assignment *> assignmentEffect;
	list <const comparison *> comparisonPrecondition;



	MyPartialOperator() {}
	void prepare (MyOperator *op, const proposition *prop);
	void prepare (MyOperator *op, const assignment *asgn);
	void prepare (MyOperator *op, const comparison *cmp);
	void grounding();
	void grounding (map <string, MyType *>::iterator it);

	bool operator == (const MyPartialOperator &a) const{
		if (op != a.op) return false;
		if (argument.size() != a.argument.size()) return false;
		map <int, MyType *>::iterator it1, itEnd, it2;
		it1 = argument.begin();
		it2 = a.argument.begin();
		itEnd = argument.end();
		for (; it1 != itEnd; ++it1, ++it2){
			if (it1->second != it2->second) return false;
		}
		return true;
	}

};



class MyPartialAction {
public:

	bool isValid;

	int id;

	enum propositionKind { E_PRECONDITION, E_ADD_EFFECT, E_DELETE_EFFECT};
	list <MyLiftedProposition *> precondition;
	list <MyLiftedProposition *> addEffect;
	list <MyLiftedProposition *> deleteEffect;

	map <func_term *, PNE *> variables;


	MyPartialOperator *partialOperator;

	MyOperator *op;

	map <string, MyObject *> objects;
	map <string, int> unificationId;

	MyPartialAction() {}

	void prepare (MyOperator *op, MyPartialOperator *partialOperator, map <string, MyObject*> &objects, map <string, int> &unificationId, int id);
	void preparePropositionList (list <const proposition *> &liftedList, list <MyLiftedProposition *> &instantiatedList, propositionKind kind);
	void findVariables(const expression *exp);

	virtual ~MyPartialAction();
};

} /* namespace mdbr */
#endif /* MYPARTIALACTION_H_ */

