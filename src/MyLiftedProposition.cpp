/*
 * MyLiftedProposition.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#include "MyLiftedProposition.h"
#include "MyProblem.h"
#include "Utilities.h"
#include "VALfiles/parsing/ptree.h"

using namespace VAL;

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
	IdOfUnification.clear();
	grounding(argument.begin());
}

void MyPartialOperator::grounding(map <string, MyType *>::iterator it){
	if (it == argument.end()){
		int partialActionId = myProblem.nPartialActions ++;
		myProblem.partialAction.push_back(MyPartialAction());
		myProblem.partialAction.rbegin()->prepare(pKind, op, this, selectedObject, IdOfUnification, partialActionId);
		return;
	}
	map <string, MyType *>::iterator nextIt;
	nextIt = it;
	++nextIt;
	int nObjects = it->second->objects.size();
	for (int i = 0; i < nObjects; ++i){
		selectedObject[it->first] = it->second->objects[i];
		IdOfUnification [it->first] = i;
		grounding(nextIt);
	}
}

void MyLiftedProposition::write (ostream &sout){
	//I think we don't need this function anymore, if you are disagree with me the correct it by yourself
//	originalLiteral->write(sout);
//	sout << " -> " << id << endl;
//
//	sout << "NEEDER: " << endl;
//	list <MyPartialAction *>::iterator it, itEnd;
//	it = needer.begin();
//	itEnd = needer.end();
//	for (; it != itEnd; ++it){
//		sout << (*it)->id << ' ';
//	}
//	sout << endl;
//
//	sout << "ADDER: " << endl;
//	it = adder.begin();
//	itEnd = adder.end();
//	for (; it != itEnd; ++it){
//		sout << (*it)->id << ' ';
//	}
//	sout << endl;
//
//	sout << "DELETER: " << endl;
//	it = deleter.begin();
//	itEnd = deleter.end();
//	for (; it != itEnd; ++it){
//		sout << (*it)->id << ' ';
//	}
//	sout << endl;
}

MyLiftedProposition::~MyLiftedProposition() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
