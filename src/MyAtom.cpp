
#include "MyAtom.h"

using namespace mdbr;

bool MyAtom::checkMutex (int layerNumber, MyAtom *otherAtom){

	list <MyGroundedAction *>::iterator it, itEnd;


	if (firstVisitedLayer < layerNumber && otherAtom->firstVisitedLayer < layerNumber){
		if (isMutex(layerNumber - 1, otherAtom) == false){
			//In the layerNumber - 1 these two proposition were not mutex, so in the layerNumber these two are not mutex too!
			return false;
		}
	}

	//check mutex of no-op action of this proposition and provider actions of other proposition
	if (this->firstVisitedLayer < layerNumber){
		it = otherAtom->provider.begin(); itEnd = otherAtom->provider.end();
		for (; it != itEnd; ++it){
			if ( !(*it)->isAtomMutex(layerNumber - 1, this)){
				return false;
			}
		}
	}


	//check mutex of no-op action of other proposition and provider actions of this proposition
	if (otherAtom->firstVisitedLayer < layerNumber){
		it = provider.begin(); itEnd = provider.end();
		for (; it != itEnd; ++it){
			if ( !(*it)->isAtomMutex(layerNumber - 1, otherAtom)){
				return false;
			}
		}
	}

	it = provider.begin(); itEnd = provider.end();
	for (; it != itEnd; ++it){

		list <MyGroundedAction *>::iterator othIt, othItEnd;
		othIt = otherAtom->provider.begin(); othItEnd = otherAtom->provider.end();
		for (; othIt != othItEnd; ++othIt){

			if ( !((*it)->isMutex(layerNumber - 1, *othIt)) ){
				return false;
			}
		}
	}

	return true;
}

bool MyAtom::isMutex (int layerNumber, MyAtom *otherAtom){
	map <MyAtom *, int>::iterator it;
	it = lastLayerMutex.find(otherAtom);
	if (it == lastLayerMutex.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;
}

void MyAtom::insertMutex (int layerNumber, MyAtom *mutexAtom){
	if (lastLayerMutex[mutexAtom] < layerNumber){
		lastLayerMutex[mutexAtom] = layerNumber;
	}
}

void MyAtom::visiting(int layerNumber, MyGroundedAction *action){
	if (firstVisitedLayer == -1 || firstVisitedLayer > layerNumber){
		firstVisitedLayer = layerNumber;
	}

	if (action == NULL){
		//It means that initial state provide this atom
		return;
	}

	provider.push_back(action);
}

MyAtom::MyAtom() {
	firstVisitedLayer = -1;
}

MyAtom::~MyAtom() {
	// TODO Auto-generated destructor stub
}

