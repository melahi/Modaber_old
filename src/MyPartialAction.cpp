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

using namespace VAL;

namespace mdbr {
void MyPartialOperator::findArgument (const parameter_symbol_list *parameter){

	parameter_symbol_list::const_iterator pIt, pItEnd;
	pIt = parameter->begin();
	pItEnd = parameter->end();
	for (int i = 0; pIt != pItEnd; ++pIt, ++i){
		const VAL::const_symbol *constSymbol = dynamic_cast <const VAL::const_symbol *> ((*pIt));
		if (constSymbol){
			var_symbol_list::iterator oIt, oItEnd;
			oIt = op->originalOperator->parameters->begin();
			oItEnd = op->originalOperator->parameters->end();
			for (; oIt != oItEnd; ++oIt){
				if ((*pIt)->type && (*pIt)->type == (*oIt)->type){
					argument.insert((*oIt));
					break;
				}
			}
		}else{
			argument.insert((*pIt));
		}
	}

}

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
		findArgument(function->getArgs());
		return;
	}

	const num_expression *number = dynamic_cast <const num_expression *> (exp);
	if (number){
		return;
	}

	CANT_HANDLE("some expression can not be handled!!!");
	return;
}

bool MyPartialOperator::isMatchingArgument (MyPartialAction *child, FastEnvironment *env){
	set <const VAL::symbol *>::const_iterator it, itEnd;
	it = argument.begin();
	itEnd = argument.end();
	for (; it != itEnd; ++it){
		if ((*(child->env))[*it]->getName() != (*env)[*it]->getName()){
			return false;
		}
	}
	return true;
}


void MyPartialOperator::prepare(MyOperator *op, const proposition *prop){
	this->op =  op;
	findArgument(prop->args);
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



bool MyPartialOperator::isSubPartialOperator (const MyPartialOperator &subPartialOperator){
	if (op != subPartialOperator.op) return false;
	if (argument.size() < subPartialOperator.argument.size()) return false;

	set <const VAL::symbol *>::const_iterator it, itEnd, it2;
	it = subPartialOperator.argument.begin();
	itEnd = subPartialOperator.argument.end();
	for (; it != itEnd; ++it){
		if (argument.find(*it) == argument.end()) return false;
	}
	return true;
}

template <class T>
void appendList (list <T> &destination, list <T> &source){
	typename list <T>::const_iterator sIt, sItEnd;
	initializeIterator(sIt, sItEnd, source);
	for (; sIt != sItEnd; ++sIt){
		destination.push_back(*sIt);
	}
}

void MyPartialOperator::mergSubPartialOperator ( MyPartialOperator &subPartialOperator){
	appendList <const proposition *> (precondition, subPartialOperator.precondition);
	appendList <const proposition *> (addEffect, subPartialOperator.addEffect);
	appendList <const proposition *> (deleteEffect, subPartialOperator.deleteEffect);
	appendList <const comparison *> (comparisonPrecondition, subPartialOperator.comparisonPrecondition);
	appendList <const assignment *> (assignmentEffect, subPartialOperator.assignmentEffect);
}

void MyPartialOperator::consideringAnAction (instantiatedOp *action){
	int childrenSize = child.size();
	FastEnvironment *env = action->getEnv();
	for (int i = 0; i < childrenSize; ++i){
		if (isMatchingArgument(child[i], env)){
			return;
		}
	}
	int partialActionId = myProblem.nPartialActions;
	myProblem.nPartialActions++;
	myProblem.partialAction.push_back(MyPartialAction());
	myProblem.partialAction.rbegin()->prepare(this, env, partialActionId);
	child.push_back(&(*myProblem.partialAction.rbegin()));
}

void MyPartialOperator::write(ostream &sout){
	sout << "("<< op->originalOperator->name->getName();
	set <const VAL::symbol *>::iterator it, itEnd;
	FOR_ITERATION(it, itEnd, argument){
		sout << " " << (*it)->getName() << "," << *it;
	}
	sout << ")";
}

bool MyPartialAction::isArgumentsConflicted (MyPartialAction *otherAction, const VAL::symbol * commonSymbol){
	return ((*env)[commonSymbol]->getName() != (*(otherAction->env))[commonSymbol]->getName());
}

void MyPartialAction::prepare (MyPartialOperator *partialOperator, FastEnvironment *env, int id){
	this->id = id;
	this->partialOperator = partialOperator;
	this->isValid = true;
	this->env = env;
	preparePropositionList(partialOperator->precondition, precondition, E_PRECONDITION);
	preparePropositionList(partialOperator->addEffect, addEffect, E_ADD_EFFECT);
	preparePropositionList(partialOperator->deleteEffect, deleteEffect, E_DELETE_EFFECT);

	findModifyingVariable();
}

void MyPartialAction::preparePropositionList (list <const proposition *> &liftedList, list <MyProposition *> &instantiatedList, propositionKind kind){
	/* IMPORTANT: This function should first called by add effects then called by delete effects,
	 * 			  because we do some refinement for delete effects base on add effects!!!
	 */
	list <const proposition* >::iterator lftIt, lftItEnd;
	lftIt = liftedList.begin();
	lftItEnd = liftedList.end();
	for (; lftIt != lftItEnd; ++lftIt){
		Literal lit(*lftIt, env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2 == NULL){
			isValid = false;
			return;
		}

		if (kind == E_DELETE_EFFECT){
			/*
			 * If an action adds some proposition and deletes the same proposition
			 * we don't considering the deleting effect!
			 * According to the Robinson's Thesis (first paragraph of page 26)
			 */
			bool isAddedBefore = false;

			list <MyProposition *>::iterator addIt, addItEnd;
			addIt = addEffect.begin();
			addItEnd = addEffect.end();
			for (; addIt != addItEnd; ++addIt){
				if ((*addIt)->originalLiteral->getStateID() == lit2->getStateID()){
					isAddedBefore = true;
					break;
				}
			}
			if (isAddedBefore){
				continue;
			}
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


void MyPartialAction::findModifyingVariable(){
	list <const assignment *>::iterator it, itEnd;
	initializeIterator(it, itEnd, partialOperator->assignmentEffect);
	for (; it != itEnd; ++it){
		PNE pne((*it)->getFTerm(), env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2 && pne2->getStateID() != -1){
			myProblem.variables[pne2->getStateID()].modifier.push_back(this);
		}
	}
}


void MyPartialAction::write (ostream &sout){
	sout << "("<< partialOperator->op->originalOperator->name->getName();
	set <const VAL::symbol *>::iterator it, itEnd;
	initializeIterator(it, itEnd, partialOperator->argument);
	for (; it != itEnd; ++it){
		sout << " " << ((*env)[(*it)])->getName();
	}
	sout << ")";
}




MyPartialAction::~MyPartialAction() {
}

} /* namespace mdbr */
