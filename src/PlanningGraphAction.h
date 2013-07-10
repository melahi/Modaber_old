/*
 * PlanningGraphAction.h
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#ifndef PLANNINGGRAPHACTION_H_
#define PLANNINGGRAPHACTION_H_

#include <list>
#include <set>
#include "MyAnalyzer.h"
#include "PlanningGraphProposition.h"

using namespace std;

class PlanningGraphProposition;

class PlanningGraphAction {
public:

	int firstVisitedLayer;

	int actionId;

	set < int > permanentMutex;

	list < set <int> > mutexInLayer;
	list < set <int> > propositionMutexInLayer;

	list < PlanningGraphProposition *> precondition;

	bool checkPropositionMutex (int layerNumber, PlanningGraphProposition *proposition);

	bool isPropositionMutex (int layerNumber, PlanningGraphProposition *proposition);

	bool isMutex (int layerNumber, PlanningGraphAction *otherAction);

	bool checkLayerMutex (int layerNumber, PlanningGraphAction *otherAction);

	bool checkPermanentMutex (PlanningGraphAction *otherAction);

	void insertMutex (int layerNumber, int otherAcitonId);
	void insertPropositionMutex (int layerNumber, int otherAcitonId);

	void computePermanentMutex (int actionId, MyAnalyzer *myAnalyzer);

	PlanningGraphAction();
	virtual ~PlanningGraphAction();
};

#endif /* PLANNINGGRAPHACTION_H_ */
