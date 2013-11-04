/*
 * MyPartialAction.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#include "MyPartialAction.h"
#include "MyLiftedProposition.h"
#include "Utilities.h"
#include "MyProblem.h"

#include <vector>
using namespace std;

namespace mdbr {


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

void MyPartialAction::preparePropositionList (list <const proposition *> &liftedList, list <MyLiftedProposition *> &instantiatedList, propositionKind kind){
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
			instantiatedList.push_back(&(myProblem.liftedPropositions[lit2->getStateID()]));
			if (kind == E_PRECONDITION){
//				myProblem.liftedPropositions[lit2->getStateID()].needer.push_back(this);
			}else if (kind == E_ADD_EFFECT){
				myProblem.liftedPropositions[lit2->getStateID()].adder.push_back(this);
			}else{
				myProblem.liftedPropositions[lit2->getStateID()].deleter.push_back(this);
			}
		}
	}
}



MyPartialAction::~MyPartialAction() {
}

} /* namespace mdbr */
