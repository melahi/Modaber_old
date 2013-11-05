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






MyAction::MyAction(): valAction(NULL), firstVisitedLayer(-1) {}

bool MyAction::isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber){
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

		if ( isVisited(myProblem.propositions[lit2->getStateID()].firstVisitedLayer, layerNumber) ){
			return true;
		}
		return false;
	}

	const comparison *comp = dynamic_cast<const comparison*> (precondition);

	if (comp){
		return true;
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


bool MyAction::isApplicable(int layerNumber){
	return (isPreconditionSatisfied(valAction->forOp()->precondition, valAction->getEnv(), layerNumber));
}

void MyAction::applyAction(int layerNumber) {

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

bool MyAction::isDynamicallyMutex(int layerNumber, MyAction *otherAction){

	map <MyAction *, int>::iterator it;
	it = lastLayerMutexivity.find(otherAction);
	if (it == lastLayerMutexivity.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;

}

bool MyAction::isMutex (int layerNumber, MyAction *otherAction){

	//Check if otherAction is statically mutex with this action or not
	if (isStaticallyMutex(otherAction)){
		return true;
	}

	//Check if otherAction is dynamically mutex with this action or not
	if (isDynamicallyMutex(layerNumber, otherAction)){
		return true;
	}

	return false;
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

bool MyAction::checkDynamicMutex(int layerNumber, MyAction *otherAction){
	/*
	 * In this function we just find dynamic mutex (It means we don't find static mutex in this function)
	 * In other words we say 2 action are dynamically mutex if there is at least one pair of mutex in their
	 * precondition (one precondition of first action be mutex with one precondition of second action).
	 */

	list <MyProposition *>::iterator it, itEnd;
	it = otherAction->propositionPrecondition.begin();
	itEnd = otherAction->propositionPrecondition.end();

	for (; it != itEnd; ++it){
		if (isPropositionMutex(layerNumber, *it)){
			//There are at least one precondition for first action and one precondition for second action which are mutex
			return true;
		}
	}

	//There is no two precondition which are mutex
	return false;
}


bool MyAction::checkDynamicPropositionMutex(int layerNumber, MyProposition *otherProposition){

	/*
	 * In this function we just find dynamic mutex (It means we don't find static mutex in this function)
	 * In other words we say a proposition and an action are dynamically mutex if there is at least one precondition which
	 * be mutex with the proposition.
	 */



	list <MyProposition *>::iterator it, itEnd;
	it = this->propositionPrecondition.begin();
	itEnd = this->propositionPrecondition.end();

	for (; it != itEnd; ++it){
		if (otherProposition->isMutex(layerNumber, *it)){
			return true;
		}
	}

	return false;
}


void MyAction::insertMutex (int layerNumber, MyAction *otherAction){
	if (lastLayerMutexivity[otherAction] < layerNumber){
		lastLayerMutexivity[otherAction] = layerNumber;
	}
}



void MyAction::insertPropositionMutex(int layerNumber, MyProposition *otherProposition){
	if (lastLayerPropositionMutexivity[otherProposition] < layerNumber){
		lastLayerPropositionMutexivity[otherProposition] = layerNumber;
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




	/*
	 * Also action which needs some precondition is mutex with action
	 * which delete it!
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
	return;
}

bool MyAction::isStaticallyMutex(MyAction *otherAction){

	if (staticMutex.find(otherAction) != staticMutex.end()){
			return true;
	}
	return false;
}

bool MyAction::isPropositionStaticallyMutex (MyProposition *otherProposition){

	/* if a proposition appeared in delete list of this action so we count it as a static mutex */

	if (deleteList.find(otherProposition) != deleteList.end()){
		return true;
	}

	return false;
}


void MyAction::write(ostream &sout){
	valAction->write(sout);
}


MyAction::~MyAction() {
	// TODO Auto-generated destructor stub
}

