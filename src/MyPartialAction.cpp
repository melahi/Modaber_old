/*
 * MyPartialAction.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#include "MyPartialAction.h"
#include "Utilities.h"
#include "MyProblem.h"
#include "VALfiles/instantiation.h"
#include <vector>
using namespace std;

namespace mdbr {


void MyPartialOperator::findTypes(const expression *exp){
	const binary_expression *binary = dynamic_cast <const binary_expression *>(exp);
	if (binary){
		findTypes(binary->getRHS());
		findTypes(binary->getLHS());
		return;
	}

	const uminus_expression *uMinus = dynamic_cast <const uminus_expression *> (exp);
	if (uMinus){
		findTypes(uMinus->getExpr());
		return;
	}

	const func_term *function = dynamic_cast <const func_term *> (exp);
	if (function){
		parameter_symbol_list::const_iterator it, itEnd;
		it = function->getArgs()->begin();
		itEnd = function->getArgs()->end();
		for (; it != itEnd; ++it){
			var_symbol_list::iterator argIt, argItEnd;
			argIt = op->originalOperator->parameters->begin();
			argItEnd = op->originalOperator->parameters->end();
			for (int j = 0; argIt != argItEnd; ++argIt, ++j){
				if ((*argIt)->getName() == (*it)->getName()){
					placement[(*it)->getName()] = j;
					argument[(*it)->getName()] = &(myProblem.types[(*it)->type]);
					break;
				}
			}
		}
		return;
	}

	const num_expression *number = dynamic_cast <const num_expression *> (exp);
	if (number){
		return;
	}

	CANT_HANDLE("some expression can not be handled!!!");
	return;
}


void MyPartialOperator::prepare(MyOperator *op, const proposition *prop){
	this->op =  op;
	parameter_symbol_list::iterator pIt, pItEnd;
	pIt = prop->args->begin();
	pItEnd = prop->args->end();
	for (int i = 0; pIt != pItEnd; ++pIt, ++i){
		var_symbol_list::iterator it, itEnd;
		it = op->originalOperator->parameters->begin();
		itEnd = op->originalOperator->parameters->end();
		for (int j = 0; it != itEnd; ++it, ++j){
			if ((*it)->getName() == (*pIt)->getName()){
				placement[(*it)->getName()] = j;
				argument[(*it)->getName()] = &(myProblem.types[(*it)->type]);
				break;
			}
		}
	}
}

void MyPartialOperator::prepare (MyOperator *op, const assignment *asgn){
	this->op = op;
	findTypes(asgn->getExpr());
	findTypes(asgn->getFTerm());
}

void MyPartialOperator::prepare (MyOperator *op, const comparison *cmp){
	this->op = op;
	findTypes(cmp->getLHS());
	findTypes(cmp->getRHS());
}



void MyPartialOperator::grounding() {
	selectedObject.clear();
	unificationId.clear();
	grounding(argument.begin());
}

void MyPartialOperator::grounding(map <string, MyType *>::iterator it){
	if (it == argument.end()){
		int partialActionId = myProblem.nPartialActions ++;
		myProblem.partialAction.push_back(MyPartialAction());
		myProblem.partialAction.rbegin()->prepare(op, this, selectedObject, unificationId, partialActionId);
		return;
	}
	map <string, MyType *>::iterator nextIt;
	nextIt = it;
	++nextIt;
	int nObjects = it->second->objects.size();
	for (int i = 0; i < nObjects; ++i){
		selectedObject[it->first] = it->second->objects[i];
		unificationId [it->first] = i;
		grounding(nextIt);
	}
}

bool MyPartialOperator::operator == (const MyPartialOperator &a) const{
	if (op != a.op) return false;
	if (argument.size() != a.argument.size()) return false;

	map <string, MyType *>::iterator it1, itEnd, it2;
	it1 = argument.begin();
	it2 = a.argument.begin();
	itEnd = argument.end();
	for (; it1 != itEnd; ++it1, ++it2){
		if (it1->second != it2->second) return false;
	}
	return true;
}


void MyPartialAction::prepare (MyOperator *op, MyPartialOperator *partialOperator, map <string, MyObject*> &objects, map <string, int> &unificationId, int id){
	this->id = id;
	this->op = op;
	this->partialOperator = partialOperator;
	this->objects = objects;
	this->unificationId = unificationId;
	this->isValid = true;
	preparePropositionList(partialOperator->precondition, precondition, E_PRECONDITION);
	preparePropositionList(partialOperator->addEffect, addEffect, E_ADD_EFFECT);
	preparePropositionList(partialOperator->deleteEffect, deleteEffect, E_DELETE_EFFECT);
	prepareComparisonList(partialOperator->comparisonPrecondition);
	prepareAssignmentList(partialOperator->assignmentEffect);
}


void MyPartialAction::findVariables(const expression *exp){
	const binary_expression *binary = dynamic_cast <const binary_expression *>(exp);
	if (binary){
		findVariables(binary->getRHS());
		findVariables(binary->getLHS());
		return;
	}

	const uminus_expression *uMinus = dynamic_cast <const uminus_expression *> (exp);
	if (uMinus){
		findVariables(uMinus->getExpr());
		return;
	}

	const func_term *function = dynamic_cast <const func_term *> (exp);
	if (function){
		parameter_symbol_list *arguments = new parameter_symbol_list();
		parameter_symbol_list::const_iterator it, itEnd;
		it = function->getArgs()->begin();
		itEnd = function->getArgs()->end();
		for (; it != itEnd; ++it){
			arguments->push_back(objects[(*it)->getName()]->originalObject);
		}
		func_term func(const_cast <func_symbol *> (function->getFunction()), arguments);

		FastEnvironment env(0);
		PNE pne(&func, &env);

		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (!pne2){
			isValid = false;
			return;
		}
		variables[function] = pne2;
		return;
	}

	const num_expression *number = dynamic_cast <const num_expression *> (exp);
	if (number){
		return;
	}

	CANT_HANDLE("some expression can not be handled!!!");
	return;

}

void MyPartialAction::prepareAssignmentList (list <const assignment *> &assignmentList){
	list <const assignment *>::iterator it, itEnd;
	it = assignmentList.begin();
	itEnd = assignmentList.end();
	for (; it != itEnd; ++it){
		findVariables((*it)->getExpr());
		findVariables((*it)->getFTerm());
		Inst::PNE *pne = variables[(*it)->getFTerm()];
		myProblem.variables[pne->getStateID()].modifier.push_back(this);
	}
}

void MyPartialAction::prepareComparisonList (list <const comparison *> &comparisonList){
	list <const comparison *>::iterator it, itEnd;
	it = comparisonList.begin();
	itEnd = comparisonList.end();
	for (; it != itEnd; ++it){
		findVariables((*it)->getLHS());
		findVariables((*it)->getRHS());
	}
}

void MyPartialAction::preparePropositionList (list <const proposition *> &liftedList, list <MyProposition *> &instantiatedList, propositionKind kind){
	list <const proposition* >::iterator lftIt, lftItEnd;
	lftIt = liftedList.begin();
	lftItEnd = liftedList.end();
	for (; lftIt != lftItEnd; ++lftIt){

		parameter_symbol_list *parameters = new parameter_symbol_list;
		parameter_symbol_list::const_iterator it, itEnd;
		it = (*lftIt)->args->begin();
		itEnd = (*lftIt)->args->end();

		for (; it != itEnd; ++it){
			parameters->push_back(objects[(*it)->getName()]->originalObject);
		}

		proposition prop((*lftIt)->head, parameters);
		FastEnvironment env(0);
		Literal lit(&prop, &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2 == NULL){
			isValid = false;
			return;
		}
		if (lit2->getStateID() != -1){
			instantiatedList.push_back(&(myProblem.propositions[lit2->getStateID()]));
			if (kind == E_PRECONDITION){
//				myProblem.propositions[lit2->getStateID()].needer.push_back(this);
			}else if (kind == E_ADD_EFFECT){
				myProblem.propositions[lit2->getStateID()].adder.push_back(this);
			}else{
				myProblem.propositions[lit2->getStateID()].deleter.push_back(this);
			}
		}
	}
}





MyPartialAction::~MyPartialAction() {
}

} /* namespace mdbr */
