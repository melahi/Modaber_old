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


bool PlanningGraphAction::checkPropositionMutex (int layerNumber, PlanningGraphProposition *proposition){
	list <set<int> >::const_iterator mutexPropositionInLayer = proposition->mutex.end();
	for (int ii = proposition->mutex.size(); ii > layerNumber; --ii, --mutexPropositionInLayer);

	list <PlanningGraphProposition *>::iterator it, itEnd;
	it = this->precondition.begin(); itEnd = this->precondition.end();

	for (; it != itEnd; ++it){
		if (mutexPropositionInLayer->find((*it)->propositionId) != mutexPropositionInLayer->end()){
			return true;
		}
	}

	return false;
}

bool PlanningGraphAction::isPropositionMutex (int layerNumber, PlanningGraphProposition *proposition){
	list < set <int> >::iterator it = propositionMutexInLayer.end();
	for (int i = propositionMutexInLayer.size(); i > layerNumber; --i, --it);

	if (it->find(proposition->propositionId) != it->end()){
		return true;
	}
	return false;
}

bool PlanningGraphAction::isMutex (int layerNumber, PlanningGraphAction *otherAction){
	list < set <int> >::iterator it;
	it = mutexInLayer.end();
	for (int i = propositionMutexInLayer.size(); i > layerNumber; --i, --it);

	if (it->find(otherAction->actionId) != it->end()){
		return true;
	}
	if (permanentMutex.find(otherAction->actionId) != permanentMutex.end()){
		return true;
	}
	return false;
}



bool PlanningGraphAction::checkLayerMutex(int layerNumber, PlanningGraphAction *otherAction){
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

bool PlanningGraphAction::checkPermanentMutex (PlanningGraphAction *otherAction){
	return (permanentMutex.find(otherAction->actionId) != permanentMutex.end());
}


void PlanningGraphAction::insertMutex (int layerNumber, int otherActionId){

	for (int i = mutexInLayer.size(); i <= layerNumber; ++i){
		mutexInLayer.push_back(set <int>());
	}

	list < set <int> >::iterator it = mutexInLayer.end();
	for (int i = mutexInLayer.size(); i > layerNumber; --i, --it);

	it->insert(otherActionId);
}



void PlanningGraphAction::insertPropositionMutex(int layerNumber, int propositionId){

	for (int i = propositionMutexInLayer.size(); i <= layerNumber; ++i){
		propositionMutexInLayer.push_back(set <int>());
	}

	list < set <int> >::iterator it = propositionMutexInLayer.end();
	for (int i = propositionMutexInLayer.size(); i > layerNumber; --i, --it);

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
}

PlanningGraphAction::~PlanningGraphAction() {
	// TODO Auto-generated destructor stub
}

