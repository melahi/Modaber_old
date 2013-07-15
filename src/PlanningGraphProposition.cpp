
#include "PlanningGraphProposition.h"

bool PlanningGraphProposition::checkMutex (int layerNumber, PlanningGraphProposition *otherProposition){
	list < set <int> >::iterator mutexActionList;
	list <PlanningGraphAction *>::iterator it, itEnd;



	if (this->firstVisitedLayer < layerNumber && otherProposition->firstVisitedLayer < layerNumber){
		if (isMutex(layerNumber - 1, otherProposition) == false){
			//In the layerNumber - 1 these two proposition were not mutex, so in the layerNumber these two are not mutex too!
			return false;
		}
	}


	//check mutex of no-op action of this proposition and provider actions of other proposition
	if (this->firstVisitedLayer < layerNumber){
		it = otherProposition->provider.begin(); itEnd = otherProposition->provider.end();
		for (; it != itEnd; ++it){
			if ( !(*it)->isPropositionMutex(layerNumber - 1, this)){
				return false;
			}
		}
	}


	//check mutex of no-op action of other proposition and provider actions of this proposition
	if (otherProposition->firstVisitedLayer < layerNumber){
		it = provider.begin(); itEnd = provider.end();
		for (; it != itEnd; ++it){
			if ( !(*it)->isPropositionMutex(layerNumber - 1, otherProposition)){
				return false;
			}
		}
	}


	//check mutex between provider actions of both proposition
	it = provider.begin(); itEnd = provider.end();
	for (; it != itEnd; ++it){

		list <PlanningGraphAction *>::iterator othIt, othItEnd;
		othIt = otherProposition->provider.begin(); othItEnd = otherProposition->provider.end();
		for (; othIt != othItEnd; ++othIt){

			if ( !((*it)->isMutex(layerNumber - 1, *othIt)) ){
				return false;
			}
		}
	}
	return true;
}

bool PlanningGraphProposition::isMutex (int layerNumber, PlanningGraphProposition *otherProposition){
	list < set <int> >::iterator it;
	it = mutex.end();
	for (int i = mutex.size(); i > layerNumber; --i, --it);

	if (it != mutex.end() && it->find(otherProposition->propositionId) != it->end()){
		return true;
	}
	return false;
}

void PlanningGraphProposition::insertMutex (int layerNumber, int mutexPropositionId){

	for (int i = mutex.size(); i <= layerNumber; ++i){
		mutex.push_back(set <int>());
	}

	list < set <int> >::iterator it = mutex.end();
	for (int i = mutex.size(); i > layerNumber; --i, --it);

	it->insert(mutexPropositionId);
}

PlanningGraphProposition::PlanningGraphProposition() {
	firstVisitedLayer = -1;
}

PlanningGraphProposition::~PlanningGraphProposition() {
	// TODO Auto-generated destructor stub
}

