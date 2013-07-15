/*
 * PlanningGraphAction.cpp
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#include "PlanningGraphAction.h"



PlanningGraphAction::PlanningGraphAction() {
	firstVisitedLayer = -1;
}

bool PlanningGraphAction::isMutex (int layerNumber, PlanningGraphAction *otherAction){

	list < set <int> >::iterator it;
	it = mutex.end();
	for (int i = propositionMutex.size(); i > layerNumber; --i, --it);

	if (it != mutex.end() && it->find(otherAction->actionId) != it->end()){
		return true;
	}
	return false;
}

bool PlanningGraphAction::isPropositionMutex (int layerNumber, PlanningGraphProposition *proposition){
	list < set <int> >::iterator it = propositionMutex.end();
	for (int i = propositionMutex.size(); i > layerNumber; --i, --it)
		;

	if (it != propositionMutex.end() && it->find(proposition->propositionId) != it->end()){
		return true;
	}
	return false;
}

bool PlanningGraphAction::checkMutex(int layerNumber, PlanningGraphAction *otherAction){
	if (permanentMutex.find(otherAction->actionId) != permanentMutex.end()){
		return true;
	}

	list <PlanningGraphProposition *>::iterator it, itEnd;
	it = otherAction->precondition.begin(); itEnd = otherAction->precondition.end();

	for (; it != itEnd; ++it){
		if (isPropositionMutex(layerNumber, *it)){
			//There are at least one precondition for first action and one precondition for second action which are mutex
			return true;
		}
	}
	//There is no two precondition which are mutex
	return false;
}


bool PlanningGraphAction::checkPropositionMutex (int layerNumber, PlanningGraphProposition *proposition){

	if (permanentPropositionMutex.find (proposition->propositionId) != permanentPropositionMutex.end()){
		return true;
	}

	list <set<int> >::const_iterator mutexPropositionList = proposition->mutex.end();
	for (int ii = proposition->mutex.size(); ii > layerNumber; --ii, --mutexPropositionList);

	if (mutexPropositionList == proposition->mutex.end()){
		return false;
	}

	list <PlanningGraphProposition *>::iterator it, itEnd;
	it = this->precondition.begin(); itEnd = this->precondition.end();

	for (; it != itEnd; ++it){
		if (mutexPropositionList->find((*it)->propositionId) != mutexPropositionList->end()){
			return true;
		}
	}

	return false;
}


void PlanningGraphAction::insertMutex (int layerNumber, int otherActionId){

	for (int i = mutex.size(); i <= layerNumber; ++i){
		mutex.push_back(set <int>());
	}

	list < set <int> >::iterator it = mutex.end();
	for (int i = mutex.size(); i > layerNumber; --i, --it);

	it->insert(otherActionId);
}



void PlanningGraphAction::insertPropositionMutex(int layerNumber, int propositionId){

	for (int i = propositionMutex.size(); i <= layerNumber; ++i){
		propositionMutex.push_back(set <int>());
	}

	list < set <int> >::iterator it = propositionMutex.end();
	for (int i = propositionMutex.size(); i > layerNumber; --i, --it);

	it->insert(propositionId);
}


void PlanningGraphAction::computePermanentMutex (int actionId, MyAnalyzer *myAnalyzer){
	this->actionId = actionId;
	set <int>::const_iterator it, itEnd;
	it = myAnalyzer->mutexActions[actionId].begin();
	itEnd = myAnalyzer->mutexActions[actionId].end();
	for (; it != itEnd; ++it){
		permanentMutex.insert(*it);
	}

	/* compute permanent proposition mutex: for doing this we
	 * should find all delete effects of the action, and then
	 * insert them to the set of permanentPropositionMutex
	 */

	instantiatedOp *op = instantiatedOp::getInstOp(actionId);
	FastEnvironment *env = op->getEnv();

	pc_list<simple_effect*> &deleteEffectList = op->forOp()->effects->del_effects;
	pc_list<simple_effect*>::const_iterator delIt = deleteEffectList.begin();
	pc_list<simple_effect*>::const_iterator delItEnd = deleteEffectList.end();

	for (; delIt != delItEnd; ++delIt){
		Literal lit ((*delIt)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		if (lit2->getStateID() != -1){
			permanentPropositionMutex.insert(lit2->getStateID());
		}
	}
}


PlanningGraphAction::~PlanningGraphAction() {
	// TODO Auto-generated destructor stub
}

