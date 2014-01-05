/*
 * Agent.cpp
 *
 *  Created on: Aug 26, 2013
 *      Author: sadra
 */

#include "Agent.h"
#include "Utilities.h"

namespace mdbr {

bool Agent::buildingSolution(){
	fitness = -1;
	if (length < 2){
		return true;
	}
	return buildingSolution(0, length - 1);
}

bool Agent::buildingSolution(int variableId, int layerNumber){
	if (layerNumber == 0){
		return true;
	}

	if (variableId == nStateVariables){
		return buildingSolution(0, layerNumber - 1);
	}

	if (layerNumber == length - 1){

	}

	return false;
}

void Agent::increaseLength(int newLength){
	if (length >= newLength){
		CANT_HANDLE("Agent wants to decrease its size, but we don't permit it!")
		return;
	}
	length = newLength;
	fitness = -1;
	for (int i = 0; i < nStateVariables; ++i){
		stateValues[i].resize(length, NULL);
	}
}



Agent::~Agent() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
