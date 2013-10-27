/*
 * StaticMutexFinder.cpp
 *
 *  Created on: Aug 6, 2013
 *      Author: sadra
 */

#include "PreconditionFinder.h"
#include "MyProblem.h"
#include "Utilities.h"

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"


using namespace Inst;
using namespace VAL;

namespace mdbr {

void PreconditionFinder::simpleGoalAnalyzer(const proposition *prop){
	Literal lit = Literal(prop, env);
	Literal *lit2 = instantiatedOp::findLiteral(&lit);
	if (lit2->getStateID() == -1){
		return;
	}
	myAction->propositionPrecondition.push_back( &(myProblem.propositions[lit2->getStateID()]) );
	myProblem.propositions[lit2->getStateID()].userActions.push_back(myAction);
}

void PreconditionFinder::expressionAnalyzer (const expression *expr, bool isPrecondition){
	const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		expressionAnalyzer(binary->getLHS(), isPrecondition);
		expressionAnalyzer(binary->getRHS(), isPrecondition);
		return;
	}
	const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
	if (uMinus){
		expressionAnalyzer(uMinus->getExpr(), isPrecondition);
		return;
	}
	const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
	if (numExpr){
		return;
	}
	const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
	if (functionTerm){
		PNE pne = PNE(functionTerm, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (!pne2 || pne2->getStateID() == -1){
			return;
		}
		myAction->variableNeeded.insert( &(myProblem.variables[pne2->getStateID()]) );
		myProblem.variables[pne2->getStateID()].userActions.push_back(myAction);
		if (isPrecondition){
			myProblem.variables[pne2->getStateID()].visitInPrecondition = true;
		}
		return;
	}
	CANT_HANDLE("Can't handle some expression in analyzing!")
	return;
}

void PreconditionFinder::operator() (const goal *gl){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		simpleGoalAnalyzer(simple->getProp());
		return;
	}
	const comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		expressionAnalyzer(comp->getLHS(), true);
		expressionAnalyzer(comp->getRHS(), true);
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		for_each(goalList->begin(), goalList->end(), *this);
		return;
	}
	CANT_HANDLE("Can't handle some precondition in analyzing!")
	return;
}



PreconditionFinder::~PreconditionFinder() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
