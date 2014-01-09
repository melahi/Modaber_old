/*
 * MyPartialAction.h
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#ifndef MYPARTIALACTION_H_
#define MYPARTIALACTION_H_

#include "MyOperator.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include "MyAtom.h"
#include "MyObject.h"

#include <vector>
#include <string>
using namespace std;



namespace mdbr {

class MyProposition;


class MyPartialOperator{
private:
	map <string, MyObject*> selectedObject;  //A temporary vector used in grounding
	map <string, int> unificationId;  //A temporary vector used in grounding

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



	MyPartialOperator(): op(NULL) {}
	void prepare (MyOperator *op, const proposition *prop);
	void prepare (MyOperator *op, const assignment *asgn);
	void prepare (MyOperator *op, const comparison *cmp);
	void grounding();
	void grounding (map <string, MyType *>::iterator it);

	bool isSubPartialOperator (const MyPartialOperator &subPartialOperator);

	void mergSubPartialOperator (const MyPartialOperator &subPartialOperator);

};



class MyPartialAction {
public:

	bool isValid;

	int id;

	enum propositionKind { E_PRECONDITION, E_ADD_EFFECT, E_DELETE_EFFECT};
	list <MyProposition *> precondition;
	list <MyProposition *> addEffect;
	list <MyProposition *> deleteEffect;

	map <const func_term *, Inst::PNE *> variables;


	MyPartialOperator *partialOperator;

	MyOperator *op;

	map <string, MyObject *> objects;
	map <string, int> unificationId;

	MyPartialAction():isValid(true), id (-2), partialOperator(NULL), op(NULL) {}

	void prepare (MyOperator *op, MyPartialOperator *partialOperator, map <string, MyObject*> &objects, map <string, int> &unificationId, int id);
	void preparePropositionList (list <const proposition *> &liftedList, list <MyProposition *> &instantiatedList, propositionKind kind);
	void prepareAssignmentList (list <const assignment *> &assignmentList);
	void prepareComparisonList (list <const comparison *> &comparisonList);
	void findVariables(const expression *exp);

	virtual ~MyPartialAction();
};

} /* namespace mdbr */
#endif /* MYPARTIALACTION_H_ */

