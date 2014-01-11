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

bool MyAction::isApplicable(int layerNumber){
//	return (isPreconditionSatisfied(valAction->forOp()->precondition, valAction->getEnv(), layerNumber));

	list <MyProposition *>::iterator it, itEnd;
	it = preconditionList.begin();
	itEnd = preconditionList.end();
	for (; it != itEnd; ++it){
		if (!isVisited((*it)->firstVisitedLayer, layerNumber)){
			return false;
		}
	}
	return true;
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
	if (lastLayerPropositionMutexivity[otherProposition] < layerNumber){
		lastLayerPropositionMutexivity[otherProposition] = layerNumber;
	}
}


void MyAction::initialize (instantiatedOp *valAction){
	this->valAction = valAction;

	const operator_ *oper = valAction->forOp();
	FastEnvironment *env = valAction->getEnv();


	pc_list <simple_effect *>::const_iterator iter, iterEnd;

	//construction deleteList
	iter = oper->effects->del_effects.begin();
	iterEnd = oper->effects->del_effects.end();
	for (;iter != iterEnd; iter++){
		Literal lit ((*iter)->prop,env);
		const Literal *lit2 = instantiatedOp::findLiteral(&lit);
		deleteList.push_back(&(myProblem.propositions[lit2->getStateID()]) );
	}

	//construct preconditionList
	findPrecondition(oper->precondition);
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

