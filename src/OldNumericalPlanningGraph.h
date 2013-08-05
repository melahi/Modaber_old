/*
 * NumericalPlanningGraph.h
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#ifndef NUMERICALPLANNINGGRAPH_H_
#define NUMERICALPLANNINGGRAPH_H_

#include "PlanningGraphAction.h"
#include "PlanningGraphProposition.h"
#include "MyAnalyzer.h"

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"

#include <vector>

using namespace std;
using namespace VAL;

class NumericalPlanningGraph {
public:

	vector <PlanningGraphAction> actions;
	vector <PlanningGraphProposition> propositions;

	int numberOfLayers;
	int numberOfMutexesInLastLayer;

	NumericalPlanningGraph(int nPropositions, int nActions, MyAnalyzer *myAnalyzer);

	void createInitialLayer();

	bool extendOneLayer();

	bool isApplicable (int actionId, int layerNumber);

	void applyAction (int actionId, int layerNumber);

	void addSimpleEffectList (const pc_list <simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber, int actionId);

	bool isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber, int actionId);

	void print(ostream &sout);

	virtual ~NumericalPlanningGraph();
};

#endif /* NUMERICALPLANNINGGRAPH_H_ */
