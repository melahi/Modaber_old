
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
	list <MyGroundedAction *>::iterator othIt, othItEnd;
	othItEnd = otherAtom->provider.end();

	for (; it != itEnd; ++it){

		othIt = otherAtom->provider.begin();
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
	it = lastLayerMutexivity.find(otherAtom);
	if (it == lastLayerMutexivity.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;
}

void MyAtom::insertMutex (int layerNumber, MyAtom *mutexAtom){
	if (lastLayerMutexivity[mutexAtom] < layerNumber){
		lastLayerMutexivity[mutexAtom] = layerNumber;
	}
}

void MyAtom::visiting(int layerNumber, MyGroundedAction *action) {
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



bool MyValue::operator < (const MyValue &otherValue) const {
	if (variable->originalPNE->getStateID() == otherValue.variable->originalPNE->getStateID()){
		return value < otherValue.value;
	}
	return variable->originalPNE->getStateID() < otherValue.variable->originalPNE->getStateID();
}

void MyValue::write (ostream &sout){
	sout << "(";
	variable->originalPNE->write(sout);
	sout << ": " << value << ")";
}


