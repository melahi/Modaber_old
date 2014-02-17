/*
 * PlanningGraphAction.cpp
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#include "MyAction.h"
#include <ptree.h>
#include <algorithm>
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "Utilities.h"
#include "MyProblem.h"


using namespace std;
using namespace Inst;
using namespace VAL;

using namespace mdbr;





MyAction::MyAction(): valAction(NULL), firstVisitedLayer(-1), possibleEffective(false) {}

//bool MyAction::isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber){
//	const simple_goal *simple = dynamic_cast<const simple_goal *>(precondition);
//
//	if (simple){
//		if (simple->getPolarity() == E_NEG){
//			return true;
//		}
//
//		Literal lit(simple->getProp(), env);
//		Literal *lit2 = instantiatedOp::getLiteral(&lit);
//
//		if (lit2->getStateID() == -1){
//			return true;
//		}
//
//		if ( isVisited(myProblem.propositions[lit2->getStateID()].firstVisitedLayer, layerNumber) ){
//			return true;
//		}
//		return false;
//	}
//
//	const comparison *comp = dynamic_cast<const comparison*> (precondition);
//
//	if (comp){
//		return true;
//	}
//
//	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(precondition);
//	if (conjunctive){
//		const goal_list *goalList = conjunctive->getGoals();
//		goal_list::const_iterator it = goalList->begin();
//		goal_list::const_iterator itEnd = goalList->end();
//		for (; it != itEnd; it++){
//			if (!isPreconditionSatisfied(*it, env, layerNumber) ){
//				return false;
//			}
//		}
//		return true;
//	}
//	CANT_HANDLE("can't evaluate some precondition");
//	return true;
//}


void MyAction::findPrecondition (const goal *gl){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		if (simple->getPolarity() == E_NEG){
			CANT_HANDLE("We don't support negative-precondition");
			return;
		}
		Literal lit = Literal(simple->getProp(), valAction->getEnv());
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() == -1){
			return;
		}
		preconditionList.push_back( &(myProblem.propositions[lit2->getStateID()]) );

		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			findPrecondition(*it);
		}
		return;
	}

	const comparison *cmp = dynamic_cast<const comparison *>(gl);
	if (cmp){
		return;
	}
	CANT_HANDLE("can't handle some goal in findingGoalList");
}



void MyAction::findingVariablesMinimum (const expression *expr, map < int, pair <bool, bool> > &result){
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
		PNE pne (function, valAction->getEnv());
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


void MyAction::findingVariablesMaximum (const expression *expr, map < int, pair <bool, bool> > &result){
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
		PNE pne (function, valAction->getEnv());
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



double MyAction::evalMaximum (const expression *expr, map <int, pair <double, double> > &selectedValue){
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
		PNE pne (function, valAction->getEnv());
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


double MyAction::evalMinimum (const expression *expr, map <int, pair <double, double> > &selectedValue){
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
		PNE pne (function, valAction->getEnv());
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

void MyAction::constructComparisons (const comparison *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values) {
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


void MyAction::constructComparisons (const comparison *expr){
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

void MyAction::constructComparisons(const goal *gl){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			constructComparisons(*it);
		}
		return;
	}

	const comparison *cmp = dynamic_cast<const comparison *>(gl);
	if (cmp){
		constructComparisons(cmp);
		return;
	}
	CANT_HANDLE("can't handle some goal in constructing comparisons");
	return;
}


void MyAction::constructAssignmentsMaximum (const assignment *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values) {
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

		PNE pne (expr->getFTerm(), valAction->getEnv());
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







void MyAction::constructAssignmentsMaximum (const assignment *expr){
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





void MyAction::constructAssignmentsMinimum (const assignment *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values) {
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

		PNE pne (expr->getFTerm(), valAction->getEnv());
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


void MyAction::constructAssignmentsMinimum (const assignment *expr){
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






void MyAction::constructAssignments(){

	if (! valAction->forOp()->effects){
		return;
	}
	pc_list <assignment *>::iterator it, itEnd;
	FOR_ITERATION(it, itEnd, valAction->forOp()->effects->assign_effects){
		constructAssignmentsMaximum(*it);
		constructAssignmentsMinimum(*it);
	}
}




void MyAction::createMyBoundVector (map <int, pair<double, double> > &values, vector <MyBound> &result){
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

void MyAction::constructNumericalCondition(){
	unacceptablePreconditionBoundaries.clear();
	assignmentBoundaries.clear();

	constructAssignments();
	constructComparisons(valAction->forOp()->precondition);
}

bool MyAction::isApplicable(int layerNumber){
//	return (isPreconditionSatisfied(valAction->forOp()->precondition, valAction->getEnv(), layerNumber));

	list <MyProposition *>::iterator it, itEnd, it2;
	it = preconditionList.begin();
	itEnd = preconditionList.end();
	for (; it != itEnd; ++it){
		if (!isVisited((*it)->firstVisitedLayer, layerNumber)){
			return false;
		}
	}
	FOR_ITERATION(it, itEnd, preconditionList){
		FOR_ITERATION(it2, itEnd, preconditionList){
			if ((*it2)->isMutex(layerNumber, *it)){
				return false;
			}
		}
	}
	return true;
}

void MyAction::applyAction(int layerNumber) {

	if (!possibleEffective){
		return;
	}

	if (isVisited(firstVisitedLayer, layerNumber)){
		//This action has been applied before!
		return;
	}

	firstVisitedLayer = layerNumber;

	addSimpleEffectList(valAction->forOp()->effects->add_effects, valAction->getEnv(), layerNumber + 1);
}

void MyAction::addSimpleEffectList (const pc_list <simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber){
	pc_list<simple_effect*>::const_iterator it = simpleEffectList.begin();
	pc_list<simple_effect*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		myProblem.propositions[lit2->getStateID()].visiting(layerNumber, this);
	}
}

bool MyAction::isPropositionDynamicallyMutex (int layerNumber, MyProposition *otherProposition){

	map <MyProposition *, int>::iterator it;
	it = lastLayerPropositionMutexivity.find(otherProposition);
	if (it == lastLayerPropositionMutexivity.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;
}

bool MyAction::isPropositionMutex (int layerNumber, MyProposition *otherProposition){

	// Check if otherAtom is statically mutex with this action or not.
	if (isPropositionStaticallyMutex(otherProposition)){
		return true;
	}

	// Check if other atom is dynamically mutex with this action or not.
	if (isPropositionDynamicallyMutex(layerNumber, otherProposition)){
		return true;
	}
	return false;
}

bool MyAction::checkDynamicPropositionMutex(int layerNumber, MyProposition *otherProposition){

	/*
	 * In this function we just find dynamic mutex (It means we don't find static mutex in this function)
	 * In other words we say a proposition and an action are dynamically mutex if there is at least one precondition which
	 * be mutex with the proposition.
	 */



	list <MyProposition *>::iterator it, itEnd;
	it = this->preconditionList.begin();
	itEnd = this->preconditionList.end();

	for (; it != itEnd; ++it){
		if (otherProposition->isMutex(layerNumber, *it)){
			return true;
		}
	}

	return false;
}


void MyAction::insertPropositionMutex(int layerNumber, MyProposition *otherProposition){
	lastLayerPropositionMutexivity[otherProposition] = layerNumber;
}


void MyAction::initialize (instantiatedOp *valAction, int id){
	this->valAction = valAction;
	this->id = id;

	const operator_ *oper = valAction->forOp();
	FastEnvironment *env = valAction->getEnv();


	pc_list <simple_effect *>::const_iterator iter, iterEnd;

	if (oper->effects){
		iter = oper->effects->add_effects.begin();
		iterEnd = oper->effects->add_effects.end();
		for (;iter != iterEnd; iter++){
			Literal lit ((*iter)->prop,env);
			const Literal *lit2 = instantiatedOp::findLiteral(&lit);
			addList.push_back(&(myProblem.propositions[lit2->getStateID()]));
			myProblem.propositions[lit2->getStateID()].adder_groundAction.push_back(this);
		}

		//construction deleteList
		iter = oper->effects->del_effects.begin();
		iterEnd = oper->effects->del_effects.end();
		for (;iter != iterEnd; iter++){
			Literal lit ((*iter)->prop,env);
			const Literal *lit2 = instantiatedOp::findLiteral(&lit);
			list <MyProposition *>::iterator addIt, addItEnd;
			bool added = false;
			FOR_ITERATION(addIt, addItEnd, addList){
				if ((*addIt)->originalLiteral->getStateID() == lit2->getStateID()){
					added = true;
					break;
				}
			}
			if (added){
				continue;
			}
			deleteList.push_back(&(myProblem.propositions[lit2->getStateID()]) );
			myProblem.propositions[lit2->getStateID()].deleter_groundAction.push_back(this);
		}


		pc_list <assignment *>::iterator it, itEnd;
		FOR_ITERATION(it, itEnd, oper->effects->assign_effects){
			PNE pne((*it)->getFTerm(), env);
			PNE *pne2 = instantiatedOp::findPNE(&pne);
			if (pne2 && pne2->getStateID() != -1){
				myProblem.variables[pne2->getStateID()].modifier_groundAction.push_back(this);
			}
		}
	}

	//construct preconditionList
	if (oper->precondition){
		findPrecondition(oper->precondition);
	}
}

bool MyAction::isPropositionStaticallyMutex (MyProposition *otherProposition){

	/* if a proposition appeared in delete list of this action so we count it as a static mutex */
	list <MyProposition *>::iterator it, itEnd;
	it = deleteList.begin();
	itEnd = deleteList.end();
	for (; it != itEnd; ++it){
		if ((*it) == otherProposition){
			return true;
		}
	}
	return false;
}


void MyAction::write(ostream &sout){
	valAction->write(sout);
}


MyAction::~MyAction() {}

