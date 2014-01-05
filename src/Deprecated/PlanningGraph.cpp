

#include "MyProblem.h"
#include "PlanningGraph.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"


#include <fstream>
#include <string>
#include <limits>

using namespace VAL;
using namespace Inst;

using namespace std;
using namespace mdbr;


void PlanningGraph::ignoreGraph(){
	/* If this function called, Numerical Planning Graph won't constructed */
	int nActions = myProblem.actions.size();
	for (int i = 0; i < nActions; i++){
		myProblem.actions[i].firstVisitedLayer = 0;
	}

	int nProposition = myProblem.propositions.size();
	for (int i = 0; i < nProposition; i++){
		myProblem.propositions[i].firstVisitedLayer = 0;
	}

	numberOfLayers = 0;
	levelOff = true;

}

void PlanningGraph::constructingGraph (int maxNumberOfLayer /* = numeric_limits <int>::max() */){

	while (!levelOff && numberOfLayers < maxNumberOfLayer){
		levelOff = !extendOneLayer();
	}
}

void PlanningGraph::createInitialLayer(){
	if (numberOfLayers > 0 || levelOff == true){
		//Initial layer is created already
		return;
	}

	//Literals
	pc_list<simple_effect*>::const_iterator it = current_analysis->the_problem->initial_state->add_effects.begin();
	pc_list<simple_effect*>::const_iterator itEnd = current_analysis->the_problem->initial_state->add_effects.end();
	FastEnvironment env(0);

	for (; it != itEnd; it++){
		Literal lit ((*it)->prop, &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() != -1){
			myProblem.propositions[lit2->getStateID()].visiting(0, NULL);
		}
	}

	numberOfLayers = 1;
	numberOfDynamicMutexesInLastLayer = 0;

}

bool PlanningGraph::extendOneLayer(){

	//@TODO: Extending one layer takes a lot of time,
	//		 I should do something for it!

	if (numberOfLayers < 1){
		// First layer is not created yet, so we should first create it!
		createInitialLayer();
		return true;
	}

	bool canContinue = false;
	list <MyAction *> newFoundedActions;
	list <MyAction *> oldFoundedActions;

	//If an action is not visited before, check its applicability, if it can be applied, then apply it!!!
	int nActions = myProblem.actions.size();
	for (int i = 0; i < nActions; i++){
		if (!isVisited(myProblem.actions[i].firstVisitedLayer, numberOfLayers - 1)){
			if (myProblem.actions[i].isApplicable(numberOfLayers - 1)){
				myProblem.actions[i].applyAction(numberOfLayers - 1);
				newFoundedActions.push_back(&(myProblem.actions[i]));
				canContinue =  true;
			}
		}else{
			oldFoundedActions.push_back(&(myProblem.actions[i]));
		}
	}

	int tempNumberOfDynamicMutex = numberOfDynamicMutexesInLastLayer;
	numberOfDynamicMutexesInLastLayer = 0;


	cout << "New Actions: " << newFoundedActions.size() << endl;



	/*************** Finding mutex relation between no-op and other actions ***************/

	list <MyAction *>::iterator actionIt, actionItEnd;
	int nPropositions = myProblem.propositions.size();

	actionIt = newFoundedActions.begin();
	actionItEnd = newFoundedActions.end();
	for (; actionIt != actionItEnd; ++actionIt) {
		for (int i = 0; i < nPropositions; ++i){
			if (!isVisited(myProblem.propositions[i].firstVisitedLayer, numberOfLayers - 1)){
				continue;
			}
			if ((*actionIt)->isPropositionStaticallyMutex(&(myProblem.propositions[i]))){
				continue;
			}
			if ((*actionIt)->checkDynamicPropositionMutex(numberOfLayers - 1, &(myProblem.propositions[i]))){
				numberOfDynamicMutexesInLastLayer += 1;
				(*actionIt)->insertPropositionMutex(numberOfLayers - 1, &(myProblem.propositions[i]));
			}
		}
	}


	actionIt = oldFoundedActions.begin();
	actionItEnd = oldFoundedActions.end();
	for (; actionIt !=actionItEnd; ++actionIt){
		map <MyProposition *, int>::iterator dynamicPropositionMutexIt, dynamicPropositionMutexItEnd;
		dynamicPropositionMutexIt = (*actionIt)->lastLayerPropositionMutexivity.begin();
		dynamicPropositionMutexItEnd = (*actionIt)->lastLayerPropositionMutexivity.end();
		for (;dynamicPropositionMutexIt != dynamicPropositionMutexItEnd; ++dynamicPropositionMutexIt){
			if (dynamicPropositionMutexIt->second == numberOfLayers - 2){
				if ((*actionIt)->checkDynamicPropositionMutex(numberOfLayers - 1, dynamicPropositionMutexIt->first)){
					numberOfDynamicMutexesInLastLayer += 1;
					(*actionIt)->insertPropositionMutex(numberOfLayers - 1, dynamicPropositionMutexIt->first);
				}
			}
		}
	}

	actionIt = oldFoundedActions.begin();
	actionItEnd = oldFoundedActions.end();
	for (; actionIt != actionItEnd; ++actionIt) {
		for (int i = 0; i < nPropositions; ++i){
			if (myProblem.propositions[i].firstVisitedLayer == numberOfLayers - 1){
				if ((*actionIt)->isPropositionStaticallyMutex(&(myProblem.propositions[i]))){
					continue;
				}
				if ((*actionIt)->checkDynamicPropositionMutex(numberOfLayers - 1, &(myProblem.propositions[i]))){
					numberOfDynamicMutexesInLastLayer += 1;
					(*actionIt)->insertPropositionMutex(numberOfLayers - 1, &(myProblem.propositions[i]));
				}
			}
		}
	}

	/*************** End of finding mutex relation between no-op and other actions ***************/



	/*************** Finding mutex relation between actions ***************/

	actionIt = oldFoundedActions.begin();
	actionItEnd = oldFoundedActions.end();
	for (;actionIt != actionItEnd; ++actionIt){
		map <MyAction *, int>::iterator actionIt2, actionItEnd2;
		actionIt2 = (*actionIt)->lastLayerMutexivity.begin();
		actionItEnd2 = (*actionIt)->lastLayerMutexivity.end();
		for (; actionIt2 != actionItEnd2; ++actionIt2){
			if (actionIt2->second == numberOfLayers - 2 && (*actionIt)->checkDynamicMutex(numberOfLayers - 1, actionIt2->first)){
				numberOfDynamicMutexesInLastLayer += 1;
				(*actionIt)->insertMutex(numberOfLayers - 1, actionIt2->first);
				actionIt2->first->insertMutex(numberOfLayers - 1, *actionIt);
			}
		}
	}

	actionIt = newFoundedActions.begin();
	actionItEnd = newFoundedActions.end();
	for (;actionIt != actionItEnd; ++actionIt){
		list<MyAction*>::iterator actionIt2, actionItEnd2;
		actionIt2 = oldFoundedActions.begin();
		actionItEnd2 = oldFoundedActions.end();
		for (;actionIt2 != actionItEnd2; ++actionIt2){
			if ((*actionIt)->isStaticallyMutex(*actionIt2)){
				continue;
			}
			if ((*actionIt)->checkDynamicMutex(numberOfLayers - 1, *actionIt2)){
				numberOfDynamicMutexesInLastLayer += 1;
				(*actionIt)->insertMutex(numberOfLayers - 1, *actionIt2);
				(*actionIt2)->insertMutex(numberOfLayers - 1, *actionIt);
			}
		}

		actionIt2 = newFoundedActions.begin();
		for (; actionIt2 != actionIt; ++actionIt2){
			if ((*actionIt)->isStaticallyMutex(*actionIt2)){
				continue;
			}
			if ((*actionIt)->checkDynamicMutex(numberOfLayers - 1, *actionIt2)){
				numberOfDynamicMutexesInLastLayer += 1;
				(*actionIt)->insertMutex(numberOfLayers - 1, *actionIt2);
				(*actionIt2)->insertMutex(numberOfLayers - 1, *actionIt);
			}
		}
	}

	//Find proposition mutex for new layer
	for (int i = 0; i < nPropositions; ++i){
		if (!isVisited(myProblem.propositions[i].firstVisitedLayer, numberOfLayers))
			continue;
		for (int j = 0; j < i; ++j){
			if (!isVisited(myProblem.propositions[j].firstVisitedLayer, numberOfLayers))
				continue;
			if (myProblem.propositions[i].checkMutex(numberOfLayers, &(myProblem.propositions[j]))){
				numberOfDynamicMutexesInLastLayer += 1;
				myProblem.propositions[i].insertMutex(numberOfLayers, &(myProblem.propositions[j]));
				myProblem.propositions[j].insertMutex(numberOfLayers, &(myProblem.propositions[i]));
			}
		}
	}


	numberOfLayers++;


	if (numberOfDynamicMutexesInLastLayer < tempNumberOfDynamicMutex){
		canContinue = true;
	}
	return canContinue;
}

PlanningGraph::PlanningGraph(){

	numberOfLayers = 0;
	levelOff = false;
	createInitialLayer();

}

PlanningGraph::~PlanningGraph() {
	// TODO Auto-generated destructor stub
}



