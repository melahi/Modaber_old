
#include "MyAtom.h"
#include "Utilities.h"
using namespace mdbr;

bool MyProposition::checkMutex (int layerNumber, MyProposition *otherProposition){
	cout << "This proposition: "; originalLiteral->write(cout); cout << endl;
	cout << "Other proposition: "; otherProposition->originalLiteral->write(cout); cout << endl;

	list <MyAction *>::iterator it, itEnd;


	if (isVisited(firstVisitedLayer ,layerNumber - 1) && isVisited(otherProposition->firstVisitedLayer, layerNumber - 1)){
		if (isMutex(layerNumber - 1, otherProposition) == false){
			//In the layerNumber - 1 these two proposition were not mutex, so in the layerNumber these two are not mutex too!
			return false;
		}
	}

	//check mutex of no-op action of this proposition and provider actions of other proposition
	if (isVisited(firstVisitedLayer ,layerNumber - 1)){
		it = otherProposition->provider.begin(); itEnd = otherProposition->provider.end();
		for (; it != itEnd; ++it){
			if ( !(*it)->isPropositionMutex(layerNumber - 1, this)){
				return false;
			}
		}
	}


	//check mutex of no-op action of other proposition and provider actions of this proposition
	if (isVisited(otherProposition->firstVisitedLayer, layerNumber - 1)){
		it = provider.begin(); itEnd = provider.end();
		for (; it != itEnd; ++it){
			cout << "Provider: "; (*it)->valAction->write(cout); cout << endl;
			if ( !(*it)->isPropositionMutex(layerNumber - 1, otherProposition)){
				return false;
			}
		}
	}

	it = provider.begin(); itEnd = provider.end();
	list <MyAction *>::iterator othIt, othItEnd;
	othItEnd = otherProposition->provider.end();

	for (; it != itEnd; ++it){

		othIt = otherProposition->provider.begin();
		for (; othIt != othItEnd; ++othIt){
			if ( (*it) == (*othIt) ){
				return false;
			}
		}
	}

	return true;
}

bool MyProposition::isMutex (int layerNumber, MyProposition *otherProposition){
	map <MyProposition *, int>::iterator it;
	it = lastLayerMutexivity.find(otherProposition);
	if (it == lastLayerMutexivity.end()){
		return false;
	}
	if (it->second < layerNumber){
		return false;
	}
	return true;
}

void MyProposition::insertMutex (int layerNumber, MyProposition *mutexProposition){
	lastLayerMutexivity[mutexProposition] = layerNumber;
	mutexProposition->lastLayerMutexivity[this] = layerNumber;
}

void MyProposition::visiting(int layerNumber, MyAction *action) {
	if (!possibleEffective){
		return;
	}
	if (!isVisited(firstVisitedLayer, layerNumber)){
		firstVisitedLayer = layerNumber;
	}

	if (action == NULL){
		//It means that initial state provide this atom
		return;
	}

	provider.push_back(action);
}

void MyProposition::write(ostream &sout){
	originalLiteral->write(sout);
}

