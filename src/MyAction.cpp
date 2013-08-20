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
#include "PreconditionFinder.h"

using namespace std;
using namespace Inst;
using namespace VAL;

using namespace mdbr;






MyAction::MyAction() {
	firstVisitedLayer = -1;
}


double MyGroundedAction::evaluateExpression (const expression *expr, FastEnvironment *env){
	double ret = 0;
	//Binary expression
	const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		double left = evaluateExpression(binary->getLHS(), env);
		double right = evaluateExpression(binary->getRHS(), env);
		if (dynamic_cast <const plus_expression* > (expr)){
			ret = left + right;
		}else if (dynamic_cast<const minus_expression *> (expr)){
			ret = left - right;
		}else if (dynamic_cast<const mul_expression *> (expr)) {
			ret = left * right;
		}else if (dynamic_cast<const div_expression *> (expr)){
			ret = left / right;
		}else{
			CANT_HANDLE("binary_expression");
		}
		return ret;
	}

	//Unary Minus
	const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
	if (uMinus){
		ret = -1 * evaluateExpression(uMinus->getExpr(), env);
		return ret;
	}

	//Constant
	const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
	if (numExpr){
		ret = numExpr->double_value();
		return ret;
	}

	//Variable
	const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
	if (functionTerm){
		PNE pne = PNE(functionTerm, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2->getStateID() == -1){
			ret = myProblem.initialValue[pne2->getGlobalID()];
			return ret;
		}

		ret = variablePrecondition[pne2->getStateID()]->value;
		return ret;
	}
	CANT_HANDLE("can't handle One expression in evaluating expression");
	return 0;

}


bool MyGroundedAction::isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(precondition);

	if (simple){
		if (simple->getPolarity() == E_NEG){
			return true;
		}
		Literal lit(simple->getProp(), env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);

		if (lit2->getStateID() == -1){
			return true;
		}

		if ( isVisited(myProblem.propositions[lit2->getStateID()].firstVisitedLayer, layerNumber)){
			return true;
		}
		return false;
	}

	const comparison *comp = dynamic_cast<const comparison*> (precondition);
	if (comp){

		//FIXME the following line is just for eliminating variables from Numerical Planning Graph;
		return true;

		double left = evaluateExpression(comp->getLHS(), env);
		double right = evaluateExpression(comp->getRHS(), env);
		switch(comp->getOp()){
		case E_GREATER:
			if (left - right > EPSILON){
				return true;
			}
			break;
		case E_GREATEQ:
			if (left - right >= -EPSILON){
				return true;
			}
			break;
		case E_LESS:
			if (left - right < -EPSILON){
				return true;
			}
			break;
		case E_LESSEQ:
			if (left - right <= EPSILON){
				return true;
			}
			break;
		case E_EQUALS:
			if (fabs(left - right) <= EPSILON){
				return true;
			}
			break;
		default:
			CANT_HANDLE ("We don't know the operator kind of numerical condition!!!");
			exit(0);
		}
		return false;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(precondition);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			if (!isPreconditionSatisfied(*it, env, layerNumber) ){
				return false;
			}
		}
		return true;
	}
	CANT_HANDLE("can't evaluate some precondition");
	return true;
}


bool MyGroundedAction::isApplicable(int layerNumber){
	instantiatedOp *op = parentAction->valAction;
	FastEnvironment *env = op->getEnv();
	return (isPreconditionSatisfied(op->forOp()->precondition, env, layerNumber));
}

void MyGroundedAction::applyAction(int layerNumber) {

	if (isVisited(firstVisitedLayer, layerNumber)){
		//This action has been applied before!
		return;
	}

	firstVisitedLayer = layerNumber;

	instantiatedOp *op = parentAction->valAction;
	FastEnvironment *env = op->getEnv();
	addSimpleEffectList(op->forOp()->effects->add_effects, env, layerNumber + 1);
	addAssignmentList(op->forOp()->effects->assign_effects, env, layerNumber + 1);
}

void MyGroundedAction::addSimpleEffectList (const pc_list <simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber){
	pc_list<simple_effect*>::const_iterator it = simpleEffectList.begin();
	pc_list<simple_effect*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		myProblem.propositions[lit2->getStateID()].visiting(layerNumber, this);
	}
}



void MyGroundedAction::addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int layerNumber){
	pc_list<assignment*>::const_iterator it = assignmentEffects.begin();
	pc_list<assignment*>::const_iterator itEnd = assignmentEffects.end();

	for (; it != itEnd; ++it){

		PNE pne ( (*it)->getFTerm(), env );
		PNE *pne2 = instantiatedOp::getPNE(&pne);

		if (myProblem.variables[pne2->getStateID()].visitInPrecondition == false){
			continue;
		}

		double value = variablePrecondition[pne2->getStateID()]->value;

		double expressionValue = evaluateExpression((*it)->getExpr(), env);

		switch ((*it)->getOp()){
		case E_INCREASE:
			value += expressionValue;
			break;
		case E_DECREASE:
			value -= expressionValue;
			break;
		case E_SCALE_UP:
			value *= expressionValue;
			break;
		case E_SCALE_DOWN:
			value /= expressionValue;
			break;
		case E_ASSIGN:
			value = expressionValue;
			break;
		case E_ASSIGN_CTS:
			cerr << "Oops!!!, I don't know what is \"E_ASSIGN_CTS\"" << endl;
			exit(1);
			break;
		default:
			cerr << (*it)->getOp() << endl;
			cerr << "I think the program should never reach at this line, BTW we just was applying a numerical effect!" << endl;
			exit (1);
		}
		myProblem.variables[pne2->getStateID()].findValue(value, layerNumber, this);
	}
}


bool MyGroundedAction::isDynamicallyMutex(int layerNumber, MyGroundedAction *otherAction){

	map <MyGroundedAction *, int>::iterator it;
	it = lastLayerMutexivity.find(otherAction);
	if (it == lastLayerMutexivity.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;

}

bool MyGroundedAction::isMutex (int layerNumber, MyGroundedAction *otherAction){

	//Check if otherAction is statically mutex with this action or not
	if (parentAction->isStaticallyMutex(otherAction->parentAction)){
		return true;
	}

	//Check if otherAction is dynamically mutex with this action or not
	if (isDynamicallyMutex(layerNumber, otherAction)){
		return true;
	}

	return false;
}

bool MyGroundedAction::isAtomDynamicallyMutex (int layerNumber, MyAtom *otherAtom){

	map <MyAtom *, int>::iterator it;
	it = lastLayerAtomMutexivity.find(otherAtom);
	if (it == lastLayerAtomMutexivity.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;
}

bool MyGroundedAction::isAtomMutex (int layerNumber, MyAtom *otherAtom){

	// Check if otherAtom is statically mutex with this action or not.
	if (parentAction->isAtomStaticallyMutex(otherAtom)){
		return true;
	}

	// Check if other atom is dynamically mutex with this action or not.
	if (isAtomDynamicallyMutex(layerNumber, otherAtom)){
		return true;
	}
	return false;
}

bool MyGroundedAction::checkDynamicMutex(int layerNumber, MyGroundedAction *otherAction){
	/*
	 * In this function we just find dynamic mutex (It means we don't find static mutex in this function)
	 * In other words we say 2 action are dynamically mutex if there is at least one pair of mutex in their
	 * precondition (one precondition of first action be mutex with one precondition of second action).
	 */

	list <MyProposition *>::iterator it, itEnd;
	it = otherAction->parentAction->propositionPrecondition.begin();
	itEnd = otherAction->parentAction->propositionPrecondition.end();

	for (; it != itEnd; ++it){
		if (isAtomMutex(layerNumber, *it)){
			//There are at least one precondition for first action and one precondition for second action which are mutex
			return true;
		}
	}

	map <int, MyValue *>::iterator it1, itEnd1;
	it1 = otherAction->variablePrecondition.begin();
	itEnd1 = otherAction->variablePrecondition.end();

	for (; it1 != itEnd1; ++it1){
		if (isAtomDynamicallyMutex(layerNumber, it1->second)){
			//There are at least one precondition for first action and one precondition for second action which are mutex
			return true;
		}
	}


	//There is no two precondition which are mutex
	return false;
}


bool MyGroundedAction::checkDynamicAtomMutex (int layerNumber, MyAtom *otherAtom){

	/*
	 * In this function we just find dynamic mutex (It means we don't find static mutex in this function)
	 * In other words we say a proposition and an action are dynamically mutex if there is at least one precondition which
	 * be mutex with the proposition.
	 */



	list <MyProposition *>::iterator it, itEnd;
	it = this->parentAction->propositionPrecondition.begin();
	itEnd = this->parentAction->propositionPrecondition.end();

	for (; it != itEnd; ++it){
		if (otherAtom->isMutex(layerNumber, *it)){
			return true;
		}
	}



	map <int, MyValue *>::iterator it1, itEnd1;
	it1 = this->variablePrecondition.begin();
	itEnd1 = this->variablePrecondition.end();

	for (; it1 != itEnd1; ++it1){
		if (otherAtom->isMutex(layerNumber, it1->second)){
			return true;
		}
	}

	return false;
}


void MyGroundedAction::insertMutex (int layerNumber, MyGroundedAction *otherAction){
	if (lastLayerMutexivity[otherAction] < layerNumber){
		lastLayerMutexivity[otherAction] = layerNumber;
	}
}



void MyGroundedAction::insertAtomMutex(int layerNumber, MyAtom *otherAtom){
	if (lastLayerAtomMutexivity[otherAtom] < layerNumber){
		lastLayerAtomMutexivity[otherAtom] = layerNumber;
	}
}


void MyAction::initialize (instantiatedOp *valAction){
	this->valAction = valAction;


	const operator_ *oper = valAction->forOp();
	FastEnvironment *env = valAction->getEnv();


	pc_list <simple_effect *>::const_iterator iter, iterEnd;

	//constructing AddList
	iter = oper->effects->add_effects.begin();
	iterEnd = oper->effects->add_effects.end();
	for (;iter != iterEnd; iter++){
		Literal lit ((*iter)->prop,env);
		const Literal *lit2 = instantiatedOp::findLiteral(&lit);
		addList.push_back( &(myProblem.propositions[lit2->getStateID()]) );
		myProblem.propositions[lit2->getStateID()].adderActions.push_back(this);
	}


	//construction deleteList
	iter = oper->effects->del_effects.begin();
	iterEnd = oper->effects->del_effects.end();
	for (;iter != iterEnd; iter++){
		Literal lit ((*iter)->prop,env);
		const Literal *lit2 = instantiatedOp::findLiteral(&lit);
		deleteList.insert( &(myProblem.propositions[lit2->getStateID()]) );
		myProblem.propositions[lit2->getStateID()].deleterActions.push_back(this);
	}

	//Initialize variableModifierAction vector
	pc_list <assignment *>::const_iterator asgnIter2, asgnIterEnd2;
	asgnIter2 = oper->effects->assign_effects.begin();
	asgnIterEnd2 = oper->effects->assign_effects.end();
	for (;asgnIter2 != asgnIterEnd2; asgnIter2++){
		PNE pne ((*asgnIter2)->getFTerm(),env);
		const PNE *pne2 = instantiatedOp::findPNE(&pne);
		modifyingVariable.insert( &(myProblem.variables[pne2->getStateID()]) );
		myProblem.variables[pne2->getStateID()].modifierActions.push_back(this);
	}

	//Initialize propositionPrecondition and variableModifier vectors
	PreconditionFinder preconditionFinder(env, this);
	preconditionFinder(oper->precondition);
	asgnIter2 = oper->effects->assign_effects.begin();
	for (;asgnIter2 != asgnIterEnd2; asgnIter2++){
		preconditionFinder.expressionAnalyzer((*asgnIter2)->getExpr(), false);
		preconditionFinder.expressionAnalyzer((*asgnIter2)->getFTerm(), false);
	}
}

void MyAction::computeStaticMutex(){

	//All action which delete a proposition which added by this action, are mutex with this action
	list <MyProposition *>::iterator it1, itEnd1;
	it1 = addList.begin();
	itEnd1 = addList.end();
	for (; it1 != itEnd1; ++it1){
		list <MyAction *>::iterator it2, itEnd2;
		it2 = (*it1)->deleterActions.begin();
		itEnd2 = (*it1)->deleterActions.end();
		for (; it2 != itEnd2; ++it2){
			staticMutex.insert(*it2);
		}
	}

	//All action which add a proposition which deleted by this action, are mutex with this action
	set <MyProposition *>::iterator it3, itEnd3;
	it3 = deleteList.begin();
	itEnd3 = deleteList.end();
	for (; it3 != itEnd3; ++it3){
		list <MyAction *>::iterator it2, itEnd2;
		it2 = (*it3)->adderActions.begin();
		itEnd2 = (*it3)->adderActions.end();
		for (; it2 != itEnd2; ++it2){
			staticMutex.insert(*it2);
		}
	}



	//We don't consider effect of more than one action on a single variable, so we consider any two actions are mutex if they affect on a single variable
	set <MyVariable *>::iterator it4, itEnd4;
	it4 = modifyingVariable.begin();
	itEnd4 = modifyingVariable.end();
	for (;it4 != itEnd4; ++it4){
		list <MyAction *>::iterator it2, itEnd2;
		it2 = (*it4)->modifierActions.begin();
		itEnd2 = (*it4)->modifierActions.end();
		for (; it2 != itEnd2; ++it2){
			if (this != *it2)
				staticMutex.insert(*it2);
		}
	}


	/* All proposition in delete list and all variables in modifier variable
	 * list are mutex with this action.
	 */

	/*
	 * Also action which needs some precondition is mutex with action
	 * which modifies (delete or change the value of) it!
	 */

	it3 = deleteList.begin();
	itEnd3 = deleteList.end();
	for (; it3 != itEnd3; ++it3){
		list <MyAction *>::iterator it2, itEnd2;
		it2 = (*it3)->userActions.begin();
		itEnd2 = (*it3)->userActions.end();
		for (; it2 != itEnd2; ++it2){
			if (this != *it2)
				staticMutex.insert(*it2);
		}
	}

	it4 = modifyingVariable.begin();
	itEnd4 = modifyingVariable.end();
	for (;it4 != itEnd4; ++it4){
		list <MyAction *>::iterator it2, itEnd2;
		it2 = (*it4)->userActions.begin();
		itEnd2 = (*it4)->userActions.end();
		for (; it2 != itEnd2; ++it2){
			if (this != *it2)
				staticMutex.insert(*it2);
		}
	}
	return;
}

bool MyAction::isStaticallyMutex(MyAction *otherAction){

	if (staticMutex.find(otherAction) != staticMutex.end()){
			return true;
	}
	return false;
}

bool MyAction::isAtomStaticallyMutex (MyAtom *atom){

	/* if an atom is a proposition and it appeared in delete list of this action so we count it as a static mutex
	 * if an atom is a value and the corresponding variable of it appeared in the modified
	 */

	MyProposition *otherProposition = dynamic_cast <MyProposition *> (atom);
	MyValue *otherValue = dynamic_cast <MyValue *> (atom);

	if (otherProposition){
		if (deleteList.find(otherProposition) != deleteList.end()){
			return true;
		}
	}else{
		if (modifyingVariable.find(otherValue->variable) != modifyingVariable.end()){
			return true;
		}
	}

	return false;
}

bool MyAction::computeGroundedAction(int layerNumber){

	/*
	 * In this function all of possible grounded actions from this action is constructed.
	 * More precisely  in this function all atom needed for grounded action is prepared and
	 * check whether there exist any pair of mutex between them, and if there is no mutex
	 * a grounded action is constructed.
	 * After that we check whether the grounded action is applicable in the mentioned layer number or not
	 * if it is applicable then we add it to the collection of grounded actions
	 */



	bool foundNewGroundedAction = false;

	list <MyProposition *>::iterator it1, itEnd1;
	it1 = propositionPrecondition.begin();
	itEnd1 = propositionPrecondition.end();
	for (; it1 != itEnd1; ++it1){
		list <MyProposition *>::iterator it2;
		it2 = propositionPrecondition.begin();
		for (; it2 != it1; ++it2){
			if ( (*it1)->isMutex(layerNumber,  *it2) ) {
				return foundNewGroundedAction;
			}
		}
	}

	set <MyVariable *>::iterator it3, itEnd3;
	it3 = variableNeeded.begin();
	itEnd3 = variableNeeded.end();



/*  DEBUGGING: finding variable domains!
if (variableNeeded.size() > 0){
	write(cout);
	cout << ", meeding variables: " << endl;
	set <MyVariable *>::iterator it33;
	it33 = variableNeeded.begin();
	for(; it33 != itEnd3; ++it33){
		(*it33)->write(cout);
		cout << endl;
	}
}
*/

	for (; it3 != itEnd3; ++it3){
		(*it3)->restart();
		if ( (*it3)->isEnd() ){
			return foundNewGroundedAction;
		}
	}

	bool canContinue = true;
	while (canContinue){
		map <int, MyValue* > selectedValue;
		it3 = variableNeeded.begin();
		bool foundedMutex = false;

		for (; it3 != itEnd3 && !foundedMutex; ++it3){
			MyValue *lastSelectedValue = (*it3)->getValue();

			//check if lastValue is mutex with other selected atoms
			list <MyProposition *>::iterator it2, itEnd2;
			it2 = propositionPrecondition.begin();
			itEnd2 = propositionPrecondition.end();
			for (; it2 != itEnd2 && !foundedMutex; ++it2){
				if ( lastSelectedValue->isMutex(layerNumber,  *it2)) {
					foundedMutex = true;
				}
			}
			map <int, MyValue* >::iterator it4, itEnd4;
			it4 = selectedValue.begin();
			itEnd4 = selectedValue.end();
			for (; it4 != itEnd4 && !foundedMutex; ++it4){
				if (lastSelectedValue->isMutex(layerNumber, it4->second)){
					foundedMutex = true;
				}
			}
			selectedValue [(*it3)->originalPNE->getStateID()] = lastSelectedValue;
		}

		if (!foundedMutex){
			MyGroundedAction newGroundedAction (this, selectedValue);
			if (groundedActions.find(newGroundedAction) == groundedActions.end()){
				if (newGroundedAction.isApplicable(layerNumber)){
					visitNewGroundedAction(layerNumber, newGroundedAction);
					foundNewGroundedAction = true;
				}
			}
		}

		if (variableNeeded.size() > 0){
			set <MyVariable *>::iterator itBegin5, it5;
			it5 = variableNeeded.end();
			--it5;
			itBegin5 = variableNeeded.begin();
			while (true){
				(*it5)->next(layerNumber);
				if ((*it5)->isEnd()){
					(*it5)->restart();
					if (it5 == itBegin5){
						canContinue = false;
						break;
					}else{
						it5--;
					}
				}else{
					break;
				}
			}
		}else{
			canContinue = false;
		}
	}
	return foundNewGroundedAction;
}

void MyAction::visitNewGroundedAction(int layerNumber, const MyGroundedAction &newGroundedAction){
	if (!isVisited(firstVisitedLayer, layerNumber)){
		firstVisitedLayer = layerNumber;
	}
	groundedActions.insert(newGroundedAction);
}

void MyAction::write(ostream &sout){
	valAction->write(sout);
}


void MyGroundedAction::write(ostream &sout){
	sout << "[";
	parentAction->valAction->write(sout);

	map <int, MyValue*>::iterator it, itEnd;
	it = variablePrecondition.begin();
	itEnd = variablePrecondition.end();

	for (; it != itEnd; ++it){
		sout << ", ";
		it->second->write(sout);
	}
	sout <<"]";
}

bool MyGroundedAction::operator < (const MyGroundedAction & a) const{
	if (parentAction->valAction->getID() == a.parentAction->valAction->getID()){
		if (variablePrecondition.size() != a.variablePrecondition.size()){
			return variablePrecondition.size() < a.variablePrecondition.size();
		}
		map <int, MyValue*>::const_iterator it1, itEnd1, it2, itEnd2;
		it1 = variablePrecondition.begin();
		itEnd1 = variablePrecondition.end();
		it2 = a.variablePrecondition.begin();
		itEnd2 = a.variablePrecondition.end();
		for (; it1 != itEnd1; ++it1, ++it2){
			if (it1->second->value != it2->second->value){
				return it1->second->value < it2->second->value;
			}
		}
		return false;
	}
	return parentAction->valAction->getID() < a.parentAction->valAction->getID();
}

MyAction::~MyAction() {
	// TODO Auto-generated destructor stub
}

