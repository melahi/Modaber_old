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

void MyPartialAction::findingVariablesMinimum (const expression *expr, map < int, pair <bool, bool> > &result){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		if (dynamic_cast <const plus_expression *> (expr)){
			findingVariablesMinimum(binary->getLHS(), result);
			findingVariablesMinimum(binary->getRHS(), result);
			return;
		}else if (dynamic_cast <const minus_expression *> (expr)){
			findingVariablesMinimum(binary->getLHS(), result);
			findingVariablesMaximum(binary->getRHS(), result);
			return;
		}else {
			findingVariablesMinimum(binary->getLHS(), result);
			findingVariablesMinimum(binary->getRHS(), result);
			findingVariablesMaximum(binary->getLHS(), result);
			findingVariablesMaximum(binary->getRHS(), result);
			return;
		}
	}
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (expr);
	if (unitMinus){
		findingVariablesMaximum(unitMinus->getExpr(), result);
		return;
	}
	const num_expression *number = dynamic_cast <const num_expression *> (expr);
	if (number){
		return;
	}
	const func_term *function = dynamic_cast <const func_term *> (expr);
	if (function){
		PNE pne (function, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2 && pne2->getStateID() != -1){
			if (result.find(pne2->getStateID()) == result.end()){
				result[pne2->getStateID()].second = false;
			}
			result[pne2->getStateID()].first = true;
		}
		return;
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return;
}


void MyPartialAction::findingVariablesMaximum (const expression *expr, map < int, pair <bool, bool> > &result){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		if (dynamic_cast <const plus_expression *> (expr)){
			findingVariablesMaximum(binary->getLHS(), result);
			findingVariablesMaximum(binary->getRHS(), result);
			return;
		}else if (dynamic_cast <const minus_expression *> (expr)){
			findingVariablesMaximum(binary->getLHS(), result);
			findingVariablesMinimum(binary->getRHS(), result);
			return;
		}else{
			findingVariablesMinimum(binary->getLHS(), result);
			findingVariablesMinimum(binary->getRHS(), result);
			findingVariablesMaximum(binary->getLHS(), result);
			findingVariablesMaximum(binary->getRHS(), result);
			return;
		}
	}
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (expr);
	if (unitMinus){
		findingVariablesMinimum(unitMinus->getExpr(), result);
		return;
	}
	const num_expression *number = dynamic_cast <const num_expression *> (expr);
	if (number){
		return;
	}
	const func_term *function = dynamic_cast <const func_term *> (expr);
	if (function){
		PNE pne (function, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2 && pne2->getStateID() != -1){
			if (result.find(pne2->getStateID()) == result.end()){
				result[pne2->getStateID()].first = false;
			}
			result[pne2->getStateID()].second = true;
		}
		return;
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return;
}



double MyPartialAction::evalMaximum (const expression *expr, map <int, pair <double, double> > &selectedValue){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		double left, right;
		if (dynamic_cast <const plus_expression *> (expr)){
			left = evalMaximum(binary->getLHS(), selectedValue);
			right = evalMaximum(binary->getRHS(), selectedValue);
			return left + right;
		}else if (dynamic_cast <const minus_expression *> (expr)){
			left = evalMaximum(binary->getLHS(), selectedValue);
			right = evalMinimum(binary->getRHS(), selectedValue);
			return left - right;
		}else if (dynamic_cast <const mul_expression *> (expr)){
			left = evalMaximum(binary->getLHS(), selectedValue);
			right = evalMaximum(binary->getRHS(), selectedValue);
			double left2, right2;
			left2 = evalMinimum(binary->getLHS(), selectedValue);
			right2 = evalMinimum(binary->getRHS(), selectedValue);
			double tempRet[4], ret;
			tempRet[0] = left * right;
			tempRet[1] = left2 * right;
			tempRet[2] = left * right2;
			tempRet[3] = left2 * right2;
			ret = tempRet[0];
			for (int i = 1; i < 4; i++){
				if (tempRet[i] > ret)
					ret = tempRet[i];
			}
			return ret;
		}else if (dynamic_cast <const div_expression *> (expr)){
			left = evalMaximum(binary->getLHS(), selectedValue);
			right = evalMaximum(binary->getRHS(), selectedValue);
			double left2, right2;
			left2 = evalMinimum(binary->getLHS(), selectedValue);
			right2 = evalMinimum(binary->getRHS(), selectedValue);
			double tempRet[4], ret;
			tempRet[0] = left / right;
			tempRet[1] = left2 / right;
			tempRet[2] = left / right2;
			tempRet[3] = left2 / right2;
			ret = tempRet[0];
			for (int i = 1; i < 4; i++){
				if (tempRet[i] > ret)
					ret = tempRet[i];
			}
			return ret;
		}else {
			CANT_HANDLE("I can't handle some binary expression!!!");
		}
	}
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (expr);
	if (unitMinus){
		return -1 * evalMinimum(unitMinus->getExpr(), selectedValue);
	}
	const num_expression *number = dynamic_cast <const num_expression *> (expr);
	if (number){
		return number->double_value();
	}
	const func_term *function = dynamic_cast <const func_term *> (expr);
	if (function){
		PNE pne (function, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2 && pne2->getStateID() != -1){
			return selectedValue[pne2->getStateID()].second;
		}
		if (pne2 && pne2->getStateID() == -1){
			return myProblem.initialValue[pne2->getGlobalID()];
		}
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return 0;
}


double MyPartialAction::evalMinimum (const expression *expr, map <int, pair <double, double> > &selectedValue){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		double left, right;
		if (dynamic_cast <const plus_expression *> (expr)){
			left = evalMinimum(binary->getLHS(), selectedValue);
			right = evalMinimum(binary->getRHS(), selectedValue);
			return left + right;
		}else if (dynamic_cast <const minus_expression *> (expr)){
			left = evalMinimum(binary->getLHS(), selectedValue);
			right = evalMaximum(binary->getRHS(), selectedValue);
			return left - right;
		}else if (dynamic_cast <const mul_expression *> (expr)){
			left = evalMaximum(binary->getLHS(), selectedValue);
			right = evalMaximum(binary->getRHS(), selectedValue);
			double left2, right2;
			left2 = evalMinimum(binary->getLHS(), selectedValue);
			right2 = evalMinimum(binary->getRHS(), selectedValue);
			double tempRet[4], ret;
			tempRet[0] = left * right;
			tempRet[1] = left2 * right;
			tempRet[2] = left * right2;
			tempRet[3] = left2 * right2;
			ret = tempRet[0];
			for (int i = 1; i < 4; i++){
				if (tempRet[i] < ret)
					ret = tempRet[i];
			}
			return ret;
		}else if (dynamic_cast <const div_expression *> (expr)){
			left = evalMaximum(binary->getLHS(), selectedValue);
			right = evalMaximum(binary->getRHS(), selectedValue);
			double left2, right2;
			left2 = evalMinimum(binary->getLHS(), selectedValue);
			right2 = evalMinimum(binary->getRHS(), selectedValue);
			double tempRet[4], ret;
			tempRet[0] = left / right;
			tempRet[1] = left2 / right;
			tempRet[2] = left / right2;
			tempRet[3] = left2 / right2;
			ret = tempRet[0];
			for (int i = 1; i < 4; i++){
				if (tempRet[i] < ret)
					ret = tempRet[i];
			}
			return ret;
		}else {
			CANT_HANDLE("I can't handle some binary expression!!!");
		}
	}
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (expr);
	if (unitMinus){
		return -1 * evalMaximum(unitMinus->getExpr(), selectedValue);
	}
	const num_expression *number = dynamic_cast <const num_expression *> (expr);
	if (number){
		return number->double_value();
	}
	const func_term *function = dynamic_cast <const func_term *> (expr);
	if (function){
		PNE pne (function, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2 && pne2->getStateID() != -1){
			return selectedValue[pne2->getStateID()].first;
		}
		if (pne2 && pne2->getStateID() == -1){
			return myProblem.initialValue[pne2->getGlobalID()];
		}
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return 0;
}

void MyPartialAction::constructComparisons (const comparison *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values) {
	if (it == variables.end()){
		if (expr->getOp() == E_EQUALS){
			double lowerLeft, upperLeft, lowerRight, upperRight;
			lowerLeft = evalMinimum(expr->getLHS(), values);
			upperLeft = evalMaximum(expr->getLHS(), values);
			lowerRight = evalMinimum(expr->getRHS(), values);
			upperRight = evalMaximum(expr->getRHS(), values);
			if (lowerLeft > lowerRight){
				swap(lowerLeft, lowerRight);
				swap(upperLeft, upperRight);
			}
			if (upperLeft < lowerRight){
				vector <MyBound> unacceptableBoundaries;
				createMyBoundVector(values, unacceptableBoundaries);
				unacceptablePreconditionBoundaries.push_back(unacceptableBoundaries);
			}
			return;
		}
		double lowerLeft, upperRight;
		if (expr->getOp() == E_LESSEQ || expr->getOp() == E_LESS){
			lowerLeft = evalMinimum(expr->getLHS(), values);
			upperRight = evalMaximum(expr->getRHS(), values);
		} else {
			lowerLeft = evalMinimum(expr->getRHS(), values);
			upperRight = evalMaximum(expr->getLHS(), values);
		}
		if (expr->getOp() == E_LESS || expr->getOp() == E_GREATER){
			if (lowerLeft >= upperRight){
				vector <MyBound> unacceptableBoundaries;
				createMyBoundVector(values, unacceptableBoundaries);
				unacceptablePreconditionBoundaries.push_back(unacceptableBoundaries);
			}
		}else{
			if (lowerLeft > upperRight){
				vector <MyBound> unacceptableBoundaries;
				createMyBoundVector(values, unacceptableBoundaries);
				unacceptablePreconditionBoundaries.push_back(unacceptableBoundaries);
			}
		}
		return;
	}
	map <double, vector <int> >::iterator domainIt,domainItEnd;
	map <int, pair <bool, bool> >::iterator nextIt = it;
	++nextIt;
	FOR_ITERATION (domainIt, domainItEnd, myProblem.variables[it->first].domain){
		if (domainIt->first == infinite){
			continue;
		}
		if (it->second.first){
			values[it->first].first = domainIt->first;
		}else{
			values[it->first].first = infinite;
		}
		map <double, vector <int> >::iterator domainIt2;
		for (domainIt2 = domainIt; domainIt2 != domainItEnd; ++domainIt2){
			if (domainIt2->first == -infinite){
				continue;
			}
			if (it->second.second){
				values[it->first].second = domainIt2->first;
			}else{
				values[it->first].second = -infinite;
			}
			constructComparisons(expr, variables, nextIt, values);
			if (!(it->second.second)){
				break;
			}
		}
		if (!(it->second.first)){
			break;
		}
	}
}


void MyPartialAction::constructComparisons (const comparison *expr){
	map <int, pair <bool, bool> > variables;
	if (expr->getOp() == E_LESS || expr->getOp() == E_LESSEQ){
		findingVariablesMinimum (expr->getLHS(), variables);
		findingVariablesMaximum (expr->getRHS(), variables);
	}else if (expr->getOp() == E_GREATER || expr->getOp() == E_GREATEQ){
		findingVariablesMinimum (expr->getRHS(), variables);
		findingVariablesMaximum (expr->getLHS(), variables);
	}else{
		findingVariablesMinimum (expr->getLHS(), variables);
		findingVariablesMaximum (expr->getRHS(), variables);
		findingVariablesMinimum (expr->getRHS(), variables);
		findingVariablesMaximum (expr->getLHS(), variables);
	}
	map <int, pair <double, double> > values;
	constructComparisons(expr, variables, variables.begin(), values);
}

void MyPartialAction::constructComparisons(){

	unacceptablePreconditionBoundaries.clear();

	list <const comparison *>::iterator it, itEnd;
	FOR_ITERATION(it, itEnd, partialOperator->comparisonPrecondition){
		constructComparisons(*it);
	}
}


void MyPartialAction::constructAssignmentsMaximum (const assignment *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values) {
	if (it == variables.end()){
		double ret = 0;
		if (expr->getOp() == E_SCALE_UP || expr->getOp() == E_SCALE_DOWN){
			double lowerExpr, upperExpr, lowerVariable, upperVariable;
			lowerExpr = evalMinimum(expr->getExpr(), values);
			upperExpr = evalMaximum(expr->getExpr(), values);
			lowerVariable = evalMinimum(expr->getFTerm(), values);
			upperVariable = evalMaximum(expr->getFTerm(), values);

			double tempRet[4];
			if (expr->getOp() == E_SCALE_UP){
				tempRet[0] = lowerExpr * lowerVariable;
				tempRet[1] = lowerExpr * upperVariable;
				tempRet[2] = upperExpr * lowerVariable;
				tempRet[3] = upperExpr * upperVariable;
			}else{
				tempRet[0] = lowerVariable / lowerExpr;
				tempRet[1] = upperVariable / lowerExpr;
				tempRet[2] = lowerVariable / upperExpr;
				tempRet[3] = upperVariable / upperExpr;
			}
			ret = tempRet[0];
			for (int i = 1; i < 4; i++){
				if (tempRet[i] > ret)
					ret = tempRet[i];
			}
		}else if (expr->getOp() == E_INCREASE){
			ret = evalMaximum(expr->getFTerm(), values);
			ret += evalMaximum(expr->getExpr(), values);
		}else if (expr->getOp() == E_DECREASE){
			ret = evalMaximum(expr->getFTerm(), values);
			ret -= evalMinimum(expr->getExpr(), values);
		}else if (expr->getOp() == E_ASSIGN){
			ret = evalMaximum(expr->getExpr(), values);
		}else{
			CANT_HANDLE("Some assignment can't be handled");
		}
		map <double, vector <int> >::iterator it;

		PNE pne (expr->getFTerm(), env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);

		it = myProblem.variables[pne2->getStateID()].domain.begin();
		while (it->first < ret){
			++it;
		}

		vector <MyBound> boundaries;
		createMyBoundVector(values, boundaries);
		pair <vector <MyBound>, MyBound > newAssignmentBoundary;
		newAssignmentBoundary.first = boundaries;
		newAssignmentBoundary.second.kind = upperBound;
		newAssignmentBoundary.second.member = it;
		assignmentBoundaries.push_back(newAssignmentBoundary);
		return;
	}
	map <double, vector <int> >::iterator domainIt,domainItEnd;
	map <int, pair <bool, bool> >::iterator nextIt = it;
	++nextIt;
	FOR_ITERATION (domainIt, domainItEnd, myProblem.variables[it->first].domain){
		if (domainIt->first == infinite){
			continue;
		}
		if (it->second.first){
			values[it->first].first = domainIt->first;
		}else{
			values[it->first].first = infinite;
		}
		map <double, vector <int> >::iterator domainIt2;
		for (domainIt2 = domainIt; domainIt2 != domainItEnd; ++domainIt2){
			if (domainIt2->first == -infinite){
				continue;
			}
			if (it->second.second){
				values[it->first].second = domainIt2->first;
			}else{
				values[it->first].second = -infinite;
			}
			constructAssignmentsMaximum (expr, variables, nextIt, values);
			if (!(it->second.second)){
				break;
			}
		}
		if (!(it->second.first)){
			break;
		}
	}
}







void MyPartialAction::constructAssignmentsMaximum (const assignment *expr){
	map <int, pair <bool, bool> > variables;

	if (expr->getOp() == E_ASSIGN){
		findingVariablesMaximum (expr->getExpr(), variables);
	}else if (expr->getOp() == E_INCREASE){
		findingVariablesMaximum(expr->getExpr(), variables);
		findingVariablesMaximum(expr->getFTerm(), variables);
	}else if (expr->getOp() == E_DECREASE){
		findingVariablesMinimum(expr->getExpr(), variables);
		findingVariablesMaximum(expr->getFTerm(), variables);
	}else{
		findingVariablesMaximum(expr->getExpr(), variables);
		findingVariablesMaximum(expr->getFTerm(), variables);
		findingVariablesMinimum(expr->getExpr(), variables);
		findingVariablesMinimum(expr->getFTerm(), variables);
	}

	map <int, pair <double, double> > values;
	constructAssignmentsMaximum (expr, variables, variables.begin(), values);
}





void MyPartialAction::constructAssignmentsMinimum (const assignment *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values) {
	if (it == variables.end()){
		double ret = 0;
		if (expr->getOp() == E_SCALE_UP || expr->getOp() == E_SCALE_DOWN){
			double lowerExpr, upperExpr, lowerVariable, upperVariable;
			lowerExpr = evalMinimum(expr->getExpr(), values);
			upperExpr = evalMaximum(expr->getExpr(), values);
			lowerVariable = evalMinimum(expr->getFTerm(), values);
			upperVariable = evalMaximum(expr->getFTerm(), values);

			double tempRet[4];
			if (expr->getOp() == E_SCALE_UP){
				tempRet[0] = lowerExpr * lowerVariable;
				tempRet[1] = lowerExpr * upperVariable;
				tempRet[2] = upperExpr * lowerVariable;
				tempRet[3] = upperExpr * upperVariable;
			}else{
				tempRet[0] = lowerVariable / lowerExpr;
				tempRet[1] = upperVariable / lowerExpr;
				tempRet[2] = lowerVariable / upperExpr;
				tempRet[3] = upperVariable / upperExpr;
			}
			ret = tempRet[0];
			for (int i = 1; i < 4; i++){
				if (tempRet[i] < ret)
					ret = tempRet[i];
			}
		}else if (expr->getOp() == E_INCREASE){
			ret = evalMinimum(expr->getFTerm(), values);
			ret += evalMinimum(expr->getExpr(), values);
		}else if (expr->getOp() == E_DECREASE){
			ret = evalMinimum(expr->getFTerm(), values);
			ret -= evalMaximum(expr->getExpr(), values);
		}else if (expr->getOp() == E_ASSIGN){
			ret = evalMinimum(expr->getExpr(), values);
		}else{
			CANT_HANDLE("Some assignment can't be handled");
		}
		map <double, vector <int> >::iterator it;

		PNE pne (expr->getFTerm(), env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);

		it = myProblem.variables[pne2->getStateID()].domain.end();
		--it;
		while (it->first > ret){
			--it;
		}

		vector <MyBound> boundaries;
		createMyBoundVector(values, boundaries);
		pair <vector <MyBound>, MyBound > newAssignmentBoundary;
		newAssignmentBoundary.first = boundaries;
		newAssignmentBoundary.second.kind = lowerBound;
		newAssignmentBoundary.second.member = it;
		assignmentBoundaries.push_back(newAssignmentBoundary);
		return;
	}
	map <double, vector <int> >::iterator domainIt,domainItEnd;
	map <int, pair <bool, bool> >::iterator nextIt = it;
	++nextIt;
	FOR_ITERATION (domainIt, domainItEnd, myProblem.variables[it->first].domain){
		if (domainIt->first == infinite){
			continue;
		}
		if (it->second.first){
			values[it->first].first = domainIt->first;
		}else{
			values[it->first].first = infinite;
		}
		map <double, vector <int> >::iterator domainIt2;
		for (domainIt2 = domainIt; domainIt2 != domainItEnd; ++domainIt2){
			if (domainIt2->first == -infinite){
				continue;
			}
			if (it->second.second){
				values[it->first].second = domainIt2->first;
			}else{
				values[it->first].second = -infinite;
			}
			constructAssignmentsMinimum (expr, variables, nextIt, values);
			if (!(it->second.second)){
				break;
			}
		}
		if (!(it->second.first)){
			break;
		}
	}
}


void MyPartialAction::constructAssignmentsMinimum (const assignment *expr){
	map <int, pair <bool, bool> > variables;

	if (expr->getOp() == E_ASSIGN){
		findingVariablesMinimum (expr->getExpr(), variables);
	}else if (expr->getOp() == E_INCREASE){
		findingVariablesMinimum(expr->getExpr(), variables);
		findingVariablesMinimum(expr->getFTerm(), variables);
	}else if (expr->getOp() == E_DECREASE){
		findingVariablesMaximum(expr->getExpr(), variables);
		findingVariablesMinimum(expr->getFTerm(), variables);
	}else{
		findingVariablesMaximum(expr->getExpr(), variables);
		findingVariablesMaximum(expr->getFTerm(), variables);
		findingVariablesMinimum(expr->getExpr(), variables);
		findingVariablesMinimum(expr->getFTerm(), variables);
	}

	map <int, pair <double, double> > values;
	constructAssignmentsMinimum(expr, variables, variables.begin(), values);
}






void MyPartialAction::constructAssignments(){
	assignmentBoundaries.clear();

	list <const assignment *>::iterator it, itEnd;
	FOR_ITERATION(it, itEnd, partialOperator->assignmentEffect){
		constructAssignmentsMaximum(*it);
		constructAssignmentsMinimum(*it);
	}
}




void MyPartialAction::createMyBoundVector (map <int, pair<double, double> > &values, vector <MyBound> &result){
	result.clear();
	map <int, pair<double, double> >::iterator it, itEnd;
	FOR_ITERATION(it, itEnd, values){
		if (it->second.first != infinite){
			MyBound myBound;
			myBound.kind = lowerBound;
			myBound.member = myProblem.variables[it->first].domain.find(it->second.first);
			result.push_back(myBound);
		}
		if (it->second.second != -infinite){
			MyBound myBound;
			myBound.kind = upperBound;
			myBound.member = myProblem.variables[it->first].domain.find(it->second.second);
			result.push_back(myBound);
		}
	}
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


void MyPartialAction::constructNumericalCondition(){
	constructAssignments();
	constructComparisons();
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
