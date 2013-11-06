/*
 * MyRelation.cpp
 *
 *  Created on: Oct 10, 2013
 *      Author: sadra
 */

#include "MyComparison.h"
#include "Utilities.h"
#include "MyProblem.h"

#include "VALfiles/instantiation.h"
#include "VALfiles/parsing/ptree.h"

#include <vector>

using namespace std;
using namespace VAL;
using namespace Inst;

namespace mdbr {

void MyLiftedComparison::prepare(MyOperator *op_,const comparison *originalComparison_){
	grounded = false;
	op = op_;
	originalComparison = originalComparison_;
	if (op){
		findTypes(originalComparison_);
	}
	grounding();
}

void MyLiftedComparison::findTypes(const expression *exp){
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
			types [(*it)->getName()] = &(myProblem.types[(*it)->type]);
			var_symbol_list::iterator argIt, argItEnd;
			argIt = op->originalOperator->parameters->begin();
			argItEnd = op->originalOperator->parameters->end();
			for (int j = 0; argIt != argItEnd; ++argIt, ++j){
				if ((*argIt)->getName() == (*it)->getName()){
					placement[(*argIt)->getName()] = j;
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

void MyLiftedComparison::grounding(){
	if (grounded){
		return;
	}
	grounding(types.begin());
	grounded = true;
}

void MyLiftedComparison::grounding(map <string, MyType*>::iterator it){
	if (it == types.end()){
		if (op){
			int comparisonId = myProblem.comparisons.size();
			myProblem.comparisons.push_back(MyComparison());
			myProblem.comparisons.rbegin()->prepare(op, this, selectedObject, objectId, comparisonId);
		}else{
			myProblem.goalComparisons.push_back(MyComparison());
			myProblem.goalComparisons.rbegin()->prepare(op, this, selectedObject, objectId, -1);
		}
		return;
	}
	map<string, MyType *>::iterator nextIt;
	nextIt = it;
	++nextIt;
	vector <MyObject *>::iterator objIt, objItEnd;
	objIt = it->second->objects.begin();
	objItEnd = it->second->objects.end();
	for (int objId = 0; objIt != objItEnd; ++objIt, ++objId){
		selectedObject[it->first] = *objIt;
		objectId[it->first] = objId;
		grounding(nextIt);
	}
}

MyComparison::MyComparison() {}

void MyComparison::prepare(MyOperator *op_, MyLiftedComparison *liftedComparison_, map <string, MyObject *> &selectedObject_, map <string, int> &objectId_, int comparisonId_){
	op = op_;
	aVariableNotFounded = false;
	liftedComparison = liftedComparison_;
	selectedObject = selectedObject_;
	objectId = objectId_;
	comparisonId = comparisonId_;
	findVariables(liftedComparison->originalComparison);
	if (aVariableNotFounded){
		return;
	}
	findPossibleRanges();
}

void MyComparison::findVariables(const expression *exp){
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
		PNE *pne2;
		if (op != NULL){
			parameter_symbol_list *arguments = new parameter_symbol_list();
			parameter_symbol_list::const_iterator it, itEnd;
			it = function->getArgs()->begin();
			itEnd = function->getArgs()->end();
			for (; it != itEnd; ++it){
				arguments->push_back(selectedObject[(*it)->getName()]->originalObject);
			}

			const func_term func(const_cast <func_symbol *> (function->getFunction()), arguments);

			FastEnvironment env(0);
			PNE pne(&func, &env);

			pne2 = instantiatedOp::findPNE(&pne);
		}else{
			FastEnvironment env(0);
			PNE pne(function, &env);
			pne2 = instantiatedOp::findPNE(&pne);
		}
		if (!pne2){
			aVariableNotFounded = true;
			return;
		}
		if (pne2->getStateID() == -1){
			MyVariable *myVariable = new MyVariable();
			myCreatedVariables.push_back(myVariable);
			myVariable->domain[myProblem.initialValue[pne2->getGlobalID()]];
			variables[function] = myVariable;
		}else{
			variables[function] = &(myProblem.variables[pne2->getStateID()]);
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

void MyComparison::findPossibleRanges(){
	if (aVariableNotFounded){
		return;
	}
	selectedRanges.clear();
	possibleRanges.clear();
	findPossibleRanges(variables.begin());
}

void MyComparison::findPossibleRanges (map <const func_term *, MyVariable *>::iterator it){
	if (it == variables.end()){
		pair <list <MyRange *>, bool > result;
		map <const func_term *, MyRange *>::iterator it, itEnd;
		it = selectedRanges.begin();
		itEnd = selectedRanges.end();
		for (; it != itEnd; ++it){
			result.first.push_back(it->second);
		}
		result.second = evalute();
		possibleRanges.push_back(result);
		return;
	}
	map <const func_term *, MyVariable *>::iterator next = it;
	++next;
	if (it->second->domain.size() == 1){
		findPossibleRanges(next);
		return;
	}
	set <MyRange>::iterator it1, it1End;
	it1 = it->second->domainRange.begin();
	it1End = it->second->domainRange.end();

	for (; it1 != it1End; ++it1){
		selectedRanges[it->first] = const_cast <MyRange *> (&(*it1));
		findPossibleRanges(next);
	}

	return;
}

bool MyComparison::evalute (){
	double left, right, left2, right2;
	if (liftedComparison->originalComparison->getOp() == E_GREATER){
		left2 = maxEvalute (liftedComparison->originalComparison->getLHS());
		right = minEvalute (liftedComparison->originalComparison->getRHS());
		return (left2 > right);
	}else if (liftedComparison->originalComparison->getOp() == E_GREATEQ){
		left2 = maxEvalute (liftedComparison->originalComparison->getLHS());
		right = minEvalute (liftedComparison->originalComparison->getRHS());
		return (left2 >= right);
	}else if (liftedComparison->originalComparison->getOp() == E_LESS){
        left = minEvalute (liftedComparison->originalComparison->getLHS());
        right2 = maxEvalute (liftedComparison->originalComparison->getRHS());
        return (left < right2);
	}else if (liftedComparison->originalComparison->getOp() == E_LESSEQ){
        left = minEvalute (liftedComparison->originalComparison->getLHS());
        right2 = maxEvalute (liftedComparison->originalComparison->getRHS());
		return (left <= right2);
	}else if (liftedComparison->originalComparison->getOp() == E_EQUALS){
        left = minEvalute (liftedComparison->originalComparison->getLHS());
		left2 = maxEvalute (liftedComparison->originalComparison->getLHS());
		right = minEvalute (liftedComparison->originalComparison->getRHS());
        right2 = maxEvalute (liftedComparison->originalComparison->getRHS());
        if (left > right){
        	swap(left, right);
        	swap(left2, right2);
        }
		return (right <= left2);
	}
	CANT_HANDLE ("SOME PROBLEM IN EVALUATION!!!");
	return false;
}



double MyComparison::minEvalute (const expression *exp){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (exp);
	if (binary){
		double left, right;
		if (dynamic_cast <const plus_expression *> (exp)){
			left = minEvalute(binary->getLHS());
			right = minEvalute(binary->getRHS());
			return left + right;
		}else if (dynamic_cast <const minus_expression *> (exp)){
			left = minEvalute(binary->getLHS());
			right = maxEvalute(binary->getRHS());
			return left - right;
		}else if (dynamic_cast <const mul_expression *> (exp)){
			left = minEvalute(binary->getLHS());
			right = minEvalute(binary->getRHS());
			double left2, right2;
			left2 = maxEvalute(binary->getLHS());
			right2 = maxEvalute(binary->getRHS());
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
		}else if (dynamic_cast <const div_expression *> (exp)){
			left = minEvalute(binary->getLHS());
			right = minEvalute(binary->getRHS());
			double left2, right2;
			left2 = maxEvalute(binary->getLHS());
			right2 = maxEvalute(binary->getRHS());
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
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (exp);
	if (unitMinus){
		double ret = -1 * maxEvalute(unitMinus->getExpr());
		double ret1 = -1 * minEvalute(unitMinus->getExpr());
		if (ret1 < ret){
			return ret1;
		}
		return ret;
	}
	const num_expression *number = dynamic_cast <const num_expression *> (exp);
	if (number){
		return number->double_value();
	}
	const func_term *function = dynamic_cast <const func_term *> (exp);
	if (function){
		if (variables[function]->domain.size() == 1){
			return variables[function]->domain.begin()->first;
		}
		return selectedRanges[function]->starting;
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return 0;
}


double MyComparison::maxEvalute (const expression *exp){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (exp);
	if (binary){
		double left, right;
		if (dynamic_cast <const plus_expression *> (exp)){
			left = maxEvalute(binary->getLHS());
			right = maxEvalute(binary->getRHS());
			return left + right;
		}else if (dynamic_cast <const minus_expression *> (exp)){
			left = maxEvalute(binary->getLHS());
			right = minEvalute(binary->getRHS());
			return left - right;
		}else if (dynamic_cast <const mul_expression *> (exp)){
			left = minEvalute(binary->getLHS());
			right = minEvalute(binary->getRHS());
			double left2, right2;
			left2 = maxEvalute(binary->getLHS());
			right2 = maxEvalute(binary->getRHS());
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
		}else if (dynamic_cast <const div_expression *> (exp)){
			left = minEvalute(binary->getLHS());
			right = minEvalute(binary->getRHS());
			double left2, right2;
			left2 = maxEvalute(binary->getLHS());
			right2 = maxEvalute(binary->getRHS());
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
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (exp);
	if (unitMinus){
		double ret = -1 * maxEvalute(unitMinus->getExpr());
		double ret1 = -1 * minEvalute(unitMinus->getExpr());
		if (ret1 > ret){
			return ret1;
		}
		return ret;
	}
	const num_expression *number = dynamic_cast <const num_expression *> (exp);
	if (number){
		return number->double_value();
	}
	const func_term *function = dynamic_cast <const func_term *> (exp);
	if (function){
		if (variables[function]->domain.size() == 1){
			return variables[function]->domain.begin()->first;
		}
		return selectedRanges[function]->ending;
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return 0;
}




void MyComparison::write(ostream &sout){
//	cout << "=======================" << endl;
//	cout << comparisonId << ": " << op->originalOperator->name->getName()<< endl;
//	map <string, MyObject *>::iterator objIt, objItEnd;
//	objIt = selectedObject.begin();
//	objItEnd = selectedObject.end();
//	cout << "OBJECTS: ";
//	for (; objIt != objItEnd; ++objIt){
//		cout << objIt->second->originalObject->getName() << ' ';
//	}
//	cout << endl;
//
//	sout << possibleRanges.size() << endl;
//	list<pair<list <MyValue*>, bool> >::iterator it, itEnd;
//	it = possibleRanges.begin();
//	itEnd = possibleRanges.end();
//	for (; it != itEnd; ++it){
//		list <MyValue*>::iterator valueIt, valueItEnd;
//		valueIt = it->first.begin();
//		valueItEnd = it->first.end();
//		for (; valueIt != valueItEnd; ++valueIt){
//			sout << "-----";
//			(*valueIt)->write(sout);
//		}
//		sout << "======" << it->second << endl;
//	}
}

} /* namespace mdbr */
