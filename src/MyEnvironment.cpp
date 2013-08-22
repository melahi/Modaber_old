/*
 * MyEnvironment.cpp
 *
 *  Created on: Aug 20, 2013
 *      Author: sadra
 */

#include "MyEnvironment.h"
#include "MyProblem.h"
#include "Utilities.h"

namespace mdbr {

void MyEnvironment::prepareProbabilityMatrixOfStateVariable (int layerNumber, int variableId){
	int domainSize = myProblem.stateVariables[variableId].domain.size();
	probability[layerNumber][variableId].resize(domainSize, vector <double>(domainSize, 0));

	for (int k = 0; k < domainSize; ++k){
		double sum = 0;
		for (int j = 0; j < domainSize; ++j){
			if (j == k){
				if (myProblem.stateVariables[variableId].domain[j].firstVisitedLayer <= layerNumber){
					probability[layerNumber][variableId][k][j] = 1;
					sum += 1;
				}
				continue;
			}
			list <MyAction *>::iterator it, itEnd;
			it = myProblem.stateVariables[variableId].domain[k].providers[j].begin();
			itEnd = myProblem.stateVariables[variableId].domain[k].providers[j].end();
			for (; it != itEnd; ++it){
				if ((*it)->firstVisitedLayer <= layerNumber){
					probability[layerNumber][variableId][k][j] = 1;
					sum += 1;
					break;
				}
			}
		}
		normolizing(probability[layerNumber][variableId][k], sum);
	}
}

void MyEnvironment::prepareLayer(int layerNumber){
	int nStateVariables = myProblem.stateVariables.size();
	probability[layerNumber].resize(nStateVariables);
	for (int i = 0; i < nStateVariables; i++){
		prepareProbabilityMatrixOfStateVariable(layerNumber, i);
	}
}

void MyEnvironment::prepare(int nSignificantTimePoint){
	if (nSignificantTimePoint - 1 <= length){
		return;
	}
	probability.resize(nSignificantTimePoint - 1);
	for (; length < nSignificantTimePoint - 1; length++){
		prepareLayer(length);
	}
}

MyEnvironment::~MyEnvironment() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
