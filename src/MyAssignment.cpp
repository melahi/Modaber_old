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
	findPossibleRanges();
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

void MyAssignment::findPossibleRanges(){
	if (aVariableNotFounded){
		return;
	}
	selectedRanges.clear();
	possibleRanges.clear();
	findPossibleRanges(variables.begin());
}

void MyAssignment::findPossibleRanges (map <const func_term *, MyVariable *>::iterator it){
	if (it == variables.end()){
		pair <list <MyRange *>, MyRange *> result;
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

MyRange *MyAssignment::evalute (){

	MyVariable *returnValue = variables[liftedAssignment->originalAssignment->getFTerm()];

	double minRightHand, maxRightHand, minFinalValue, maxFinalValue;
	minRightHand = minEvalute (liftedAssignment->originalAssignment->getExpr());
	maxRightHand = maxEvalute (liftedAssignment->originalAssignment->getExpr());
	minFinalValue = minEvalute(liftedAssignment->originalAssignment->getFTerm());
	maxFinalValue = maxEvalute(liftedAssignment->originalAssignment->getFTerm());
	if (liftedAssignment->originalAssignment->getOp() == E_ASSIGN){
		minFinalValue = minEvalute (liftedAssignment->originalAssignment->getExpr());
		maxFinalValue = maxEvalute (liftedAssignment->originalAssignment->getExpr());
	}else if (liftedAssignment->originalAssignment->getOp() == E_INCREASE){
		minRightHand = minEvalute (liftedAssignment->originalAssignment->getExpr());
		maxRightHand = maxEvalute (liftedAssignment->originalAssignment->getExpr());
		minFinalValue = minEvalute(liftedAssignment->originalAssignment->getFTerm());
		maxFinalValue = maxEvalute(liftedAssignment->originalAssignment->getFTerm());
		minFinalValue += minRightHand;
		maxFinalValue += maxRightHand;
	}else if (liftedAssignment->originalAssignment->getOp() == E_DECREASE){
		minRightHand = minEvalute (liftedAssignment->originalAssignment->getExpr());
		maxRightHand = maxEvalute (liftedAssignment->originalAssignment->getExpr());
		minFinalValue = minEvalute(liftedAssignment->originalAssignment->getFTerm());
		maxFinalValue = maxEvalute(liftedAssignment->originalAssignment->getFTerm());
		minFinalValue -= maxRightHand;
		maxFinalValue -= minRightHand;
	}else if (liftedAssignment->originalAssignment->getOp() == E_SCALE_UP){
		minRightHand = minEvalute (liftedAssignment->originalAssignment->getExpr());
		maxRightHand = maxEvalute (liftedAssignment->originalAssignment->getExpr());
		minFinalValue = minEvalute(liftedAssignment->originalAssignment->getFTerm());
		maxFinalValue = maxEvalute(liftedAssignment->originalAssignment->getFTerm());
		double temp[4];
		temp[0] = minFinalValue * minRightHand;
		temp[1] = minFinalValue * maxRightHand;
		temp[2] = maxFinalValue * minRightHand;
		temp[3] = maxFinalValue * maxRightHand;
		minFinalValue = temp[0];
		maxFinalValue = temp[0];
		for (int i = 1; i < 4; ++i){
			if (minFinalValue > temp[i]){
				minFinalValue = temp[i];
			}
			if (maxFinalValue < temp[i]){
				maxFinalValue = temp[i];
			}
		}
	}else if (liftedAssignment->originalAssignment->getOp() == E_SCALE_DOWN){
		minRightHand = minEvalute (liftedAssignment->originalAssignment->getExpr());
		maxRightHand = maxEvalute (liftedAssignment->originalAssignment->getExpr());
		minFinalValue = minEvalute(liftedAssignment->originalAssignment->getFTerm());
		maxFinalValue = maxEvalute(liftedAssignment->originalAssignment->getFTerm());
		double temp[4];
		temp[0] = minFinalValue / minRightHand;
		temp[1] = minFinalValue / maxRightHand;
		temp[2] = maxFinalValue / minRightHand;
		temp[3] = maxFinalValue / maxRightHand;
		minFinalValue = temp[0];
		maxFinalValue = temp[0];
		for (int i = 1; i < 4; ++i){
			if (minFinalValue > temp[i]){
				minFinalValue = temp[i];
			}
			if (maxFinalValue < temp[i]){
				maxFinalValue = temp[i];
			}
		}
	}else{
		CANT_HANDLE ("SOME PROBLEM IN EVALUATION!!!");
	}

	MyRange a;
	a.starting = returnValue->findGreatestMinimum(minFinalValue);
	a.ending = returnValue->findLeastMaximum(maxFinalValue);
	MyRange* ret = const_cast <MyRange *> (&(*(returnValue->domainRange.find(a))));
	return ret;
}

double MyAssignment::minEvalute (const expression *exp){
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


double MyAssignment::maxEvalute (const expression *exp){
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




void MyAssignment::write(ostream &sout){
//	cout << "***********************" << endl;
//	cout << assignmentId << ": " << op->originalOperator->name->getName()<< endl;
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
//	list<pair<list <MyValue*>, MyValue*> >::iterator it, itEnd;
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
//		sout << "******"; it->second->write(sout); cout << endl;
//	}

}

} /* namespace mdbr */
