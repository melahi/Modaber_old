/*
 * PlanningGraphProposition.h
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#ifndef PLANNINGGRAPHPROPOSITION_H_
#define PLANNINGGRAPHPROPOSITION_H_


#include <list>
#include <set>
#include "PlanningGraphAction.h"

using namespace std;

class PlanningGraphAction;

class PlanningGraphProposition {
public:

	int firstVisitedLayer;

	int propositionId;

	list < set <int> > mutex;

	list <PlanningGraphAction *> provider;

	bool checkMutex (int layerNumber, PlanningGraphProposition *otherProposition);

	bool isMutex (int layerNumber, PlanningGraphProposition *otherProposition);

	void insertMutex (int layerNumber, int mutexPropositionId);

	PlanningGraphProposition();

	virtual ~PlanningGraphProposition();
};

#endif /* PLANNINGGRAPHPROPOSITION_H_ */
