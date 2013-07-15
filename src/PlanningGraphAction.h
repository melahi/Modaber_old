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
	set < int > permanentPropositionMutex;

	list < set <int> > mutex;
	list < set <int> > propositionMutex;

	list < PlanningGraphProposition *> precondition;

	bool isMutex (int layerNumber, PlanningGraphAction *otherAction);
	bool isPropositionMutex (int layerNumber, PlanningGraphProposition *proposition);

	bool checkMutex (int layerNumber, PlanningGraphAction *otherAction);
	bool checkPropositionMutex (int layerNumber, PlanningGraphProposition *proposition);

	void insertMutex (int layerNumber, int otherAcitonId);
	void insertPropositionMutex (int layerNumber, int otherAcitonId);

	void computePermanentMutex (int actionId, MyAnalyzer *myAnalyzer);

	PlanningGraphAction();
	virtual ~PlanningGraphAction();
};

#endif /* PLANNINGGRAPHACTION_H_ */
