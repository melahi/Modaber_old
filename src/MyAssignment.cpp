/*
 * MyAssignment.cpp
 *
 *  Created on: Oct 13, 2013
 *      Author: sadra
 */

#include "MyAssignment.h"
#include "Utilities.h"
#include "MyAtom.h"
#include "MyProblem.h"

#include "VALfiles/parsing/ptree.h"

using namespace VAL;

namespace mdbr {


void MyLiftedAssignment::prepare(MyOperator *op_, assignment *originalAssignment_){
	grounded = false;
	op = op_;
	originalAssignment = originalAssignment_;
	findTypes(originalAssignment_->getExpr());
	findTypes(originalAssignment_->getFTerm());
	grounding();
}

void MyLiftedAssignment::findTypes(const expression *exp){
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

void MyLiftedAssignment::grounding(){
	if (grounded){
		return;
	}
	grounding(types.begin());
	grounded = true;
}

void MyLiftedAssignment::grounding(map <string, MyType*>::iterator it){
	if (it == types.end()){
		int assignemntId = myProblem.assignments.size();
		myProblem.assignments.push_back(MyAssignment());
		myProblem.assignments.rbegin()->prepare(op, this, selectedObject, objectId, assignemntId);
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

MyAssignment::MyAssignment() {}

bool MyAssignment::isMutex (MyAssignment *other){
	if (possibleValues.size() == 0 || other->possibleValues.size() == 0){
		return false;
	}
	int idOfChangingVariable = possibleValues.begin()->second->variable->originalPNE->getStateID();

	if (idOfChangingVariable == other->possibleValues.begin()->second->variable->originalPNE->getStateID()){
		return true;
	}

	list <MyValue*>::iterator it, itEnd;
	it = other->possibleValues.begin()->first.begin();
	itEnd = other->possibleValues.begin()->first.end();
	for (; it != itEnd; ++it){
		if (idOfChangingVariable == (*it)->variable->originalPNE->getStateID()){
			return true;
		}
	}
	return false;
}

bool MyAssignment::isMutex (MyComparison *other){
	if (possibleValues.size() == 0 || other->possibleValues.size() == 0){
		return false;
	}
	if (op->id == other->op->id && objectId.size() == other->objectId.size()){
		map <string, int>::iterator it1, it2, it1End, it2End;
		it1 = objectId.begin();
		it1End = objectId.end();
		it2 = other->objectId.begin();
		it2End = other->objectId.end();
		for (; it1 != it1End; ++it1, ++it2){
			if ((it1->first != it2->first) || (it1->second != it2->second)){
				break;
			}
		}
		if (it1 == it1End){
			//both of comparison and assignment are for same action
			return false;
		}
	}

	int idOfChangingVariable = possibleValues.begin()->second->variable->originalPNE->getStateID();
	list <MyValue*>::iterator it, itEnd;
	it = other->possibleValues.begin()->first.begin();
	itEnd = other->possibleValues.begin()->first.end();
	for (; it != itEnd; ++it){
		if (idOfChangingVariable == (*it)->variable->originalPNE->getStateID()){
			return true;
		}
	}
	return false;
}

void MyAssignment::prepare(MyOperator *op_, MyLiftedAssignment *liftedAssignment_, map <string, MyObject *> &selectedObject_, map <string, int> &objectId_, int assignmentId_){
	op = op_;
	liftedAssignment = liftedAssignment_;
	selectedObject = selectedObject_;
	objectId = objectId_;
	assignmentId = assignmentId_;
	aVariableNotFounded = false;
	findVariables(liftedAssignment->originalAssignment->getExpr());
	findVariables(liftedAssignment->originalAssignment->getFTerm());
	if (aVariableNotFounded || !(myProblem.variables[variables[liftedAssignment->originalAssignment->getFTerm()]->originalPNE->getStateID()].visitInPrecondition)){
		return;
	}
	findPossibleValues();
	variables[liftedAssignment->originalAssignment->getFTerm()]->assigner.push_back(this);
}

void MyAssignment::findVariables(const expression *exp){
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
			arguments->push_back(selectedObject[(*it)->getName()]->originalObject);
		}
		func_term func(const_cast <func_symbol *> (function->getFunction()), arguments);

		FastEnvironment env(0);
		PNE pne(&func, &env);

		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (!pne2){
			aVariableNotFounded = true;
			return;
		}
		if (pne2->getStateID() == -1){
			MyVariable *myVariable = new MyVariable();
			myCreatedVariables.push_back(myVariable);
			myVariable->domain[myProblem.initialValue[pne2->getGlobalID()]].value = myProblem.initialValue[pne2->getGlobalID()];
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

void MyAssignment::findPossibleValues(){
	selectedValues.clear();
	possibleValues.clear();
	findPossibleValues(variables.begin());
}

void MyAssignment::findPossibleValues (map <const func_term *, MyVariable *>::iterator it){
	if (it == variables.end()){
		pair <list <MyValue *>, MyValue* > result;
		map <const func_term *, MyValue *>::iterator it, itEnd;
		it = selectedValues.begin();
		itEnd = selectedValues.end();
		for (; it != itEnd; ++it){
			result.first.push_back(it->second);
		}
		result.second = evalute();
		if (result.second != NULL){
			possibleValues.push_back(result);
		}
		return;
	}
	map <const func_term *, MyVariable *>::iterator next = it;
	++next;
	if (it->second->domain.size() == 1){
		findPossibleValues(next);
		return;
	}
	map <double, MyValue>::iterator it1, it1End;
	it1 = it->second->domain.begin();
	it1End = it->second->domain.end();

	for (; it1 != it1End; ++it1){
		selectedValues[it->first] = &(it1->second);
		findPossibleValues(next);
	}
	return;
}

MyValue *MyAssignment::evalute (){
	double rightHand, finalValue;
	rightHand = evalute (liftedAssignment->originalAssignment->getExpr());
	finalValue = evalute(liftedAssignment->originalAssignment->getFTerm());
	if (rightHand != undefinedValue && finalValue != undefinedValue){
		if (liftedAssignment->originalAssignment->getOp() == E_ASSIGN){
			finalValue = rightHand;
		}else if (liftedAssignment->originalAssignment->getOp() == E_INCREASE){
			finalValue += rightHand;
		}else if (liftedAssignment->originalAssignment->getOp() == E_DECREASE){
			finalValue -= rightHand;
		}else if (liftedAssignment->originalAssignment->getOp() == E_SCALE_UP){
			finalValue *= rightHand;
		}else if (liftedAssignment->originalAssignment->getOp() == E_SCALE_DOWN){
			finalValue /= rightHand;
		}else{
			CANT_HANDLE ("SOME PROBLEM IN EVALUATION!!!");
		}
	}else{
		finalValue = undefinedValue;
	}

	MyVariable *returnValue = variables[liftedAssignment->originalAssignment->getFTerm()];
	if (returnValue->domain.find(finalValue) != returnValue->domain.end()){
		return &(returnValue->domain[finalValue]);
	}
	return NULL;
}

double MyAssignment::evalute (const expression *exp){
	const binary_expression *binary = dynamic_cast <const binary_expression *> (exp);
	if (binary){
		double left, right;
		left = evalute(binary->getLHS());
		right = evalute(binary->getRHS());

		if (left == undefinedValue || right == undefinedValue){
			return undefinedValue;
		}

		if (dynamic_cast <const plus_expression *> (exp)){
			return left + right;
		}else if (dynamic_cast <const minus_expression *> (exp)){
			return left - right;
		}else if (dynamic_cast <const mul_expression *> (exp)){
			return left * right;
		}else if (dynamic_cast <const div_expression *> (exp)){
			return left / right;
		}else {
			CANT_HANDLE("I can't handle some binary expression!!!");
		}
	}
	const uminus_expression *unitMinus = dynamic_cast <const uminus_expression *> (exp);
	if (unitMinus){
		double ret = evalute(unitMinus->getExpr());
		if (ret == undefinedValue){
			return undefinedValue;
		}
		return -1 * ret;
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
		return selectedValues[function]->value;
	}
	CANT_HANDLE("I can't handle some expression!!!");
	return 0;
}



void MyAssignment::findAllMutexes(){
	list <MyAssignment>::iterator it, itEnd;
	it = myProblem.assignments.begin();
	itEnd = myProblem.assignments.end();
	for (; it != itEnd; ++it){
		if (assignmentId != it->assignmentId){
			if (isMutex(&(*it))){
				assignmentMutex.push_back(&(*it));
			}
		}
	}

	list <MyComparison>::iterator it1, it1End;
	it1 = myProblem.comparisons.begin();
	it1End = myProblem.comparisons.end();
	for (; it1 != it1End; ++it1){
		if (isMutex(&(*it1))){
			comparisonMutex.push_back(&(*it1));
		}
	}
}


} /* namespace mdbr */
