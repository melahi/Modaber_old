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




#include <vector>
#include <string>
#include <set>

using namespace std;
using namespace Inst;


namespace mdbr {

class MyProposition;
class MyPartialAction;
class MyOperator;


class MyPartialOperator{
private:
	void findTypes(const VAL::expression *exp);

	void findArgument (const parameter_symbol_list *parameter);

	bool isMatchingArgument (MyPartialAction *child, FastEnvironment *env);

public:

	MyOperator *op;
	set <const VAL::symbol *> argument;

	vector <MyPartialAction *> child;


	list <const proposition *> precondition;
	list <const proposition *> addEffect;
	list <const proposition *> deleteEffect;
	list <const assignment *> assignmentEffect;
	list <const comparison *> comparisonPrecondition;

	MyPartialOperator(): op(NULL) {}
	void prepare (MyOperator *op, const proposition *prop);
	void prepare (MyOperator *op, const assignment *asgn);
	void prepare (MyOperator *op, const comparison *cmp);

	bool isSubPartialOperator (const MyPartialOperator &subPartialOperator);

	void mergSubPartialOperator ( MyPartialOperator &subPartialOperator);

	void consideringAnAction (instantiatedOp *action);

	void write (ostream &sout);

};



class MyPartialAction {
private:

	bool isArgumentsConflicted (MyPartialAction *otherAction, const VAL::symbol *commonSymbol);

public:

	bool isValid;

	int id;

	enum propositionKind { E_PRECONDITION, E_ADD_EFFECT, E_DELETE_EFFECT};
	list <MyProposition *> precondition;
	list <MyProposition *> addEffect;
	list <MyProposition *> deleteEffect;

	FastEnvironment *env;

	MyPartialOperator *partialOperator;


	MyPartialAction():isValid(true), id (-2), partialOperator(NULL){}

	void prepare (MyPartialOperator *partialOperator, FastEnvironment *env, int id);
	void preparePropositionList (list <const proposition *> &liftedList, list <MyProposition *> &instantiatedList, propositionKind kind);

	void findModifyingVariable();


	void write (ostream &sout);

	virtual ~MyPartialAction();
};

} /* namespace mdbr */
#endif /* MYPARTIALACTION_H_ */

