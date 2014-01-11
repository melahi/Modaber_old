

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
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < nOperator; ++i){
		int nActions = myProblem.actions[i].size();
		for (int j = 0; j < nActions; j++){
			myProblem.actions[i][j].firstVisitedLayer = 0;
		}
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
	int tempNumberOfDynamicMutex = numberOfDynamicMutexesInLastLayer;
	numberOfDynamicMutexesInLastLayer = 0;

	//If an action is not visited before, check its applicability, if it can be applied, then apply it!!!
	int nOperators = myProblem.operators.size();
	for (int i = 0; i < nOperators; i++){
		int nActions = myProblem.actions[i].size();
		int lastLayer = (numberOfLayers - 1) * nOperators + i;

		list <MyAction *> newFoundedActions;
		list <MyAction *> oldFoundedActions;


		for (int j = 0; j < nActions; j++){
			if (!myProblem.actions[i][j].possibleEffective){
				continue;
			}
			if (!isVisited(myProblem.actions[i][j].firstVisitedLayer, lastLayer)){
				if (myProblem.actions[i][j].isApplicable(lastLayer)){
					myProblem.actions[i][j].applyAction(lastLayer);
					newFoundedActions.push_back(&(myProblem.actions[i][j]));
					canContinue =  true;
				}
			}else{
				oldFoundedActions.push_back(&(myProblem.actions[i][j]));
			}
		}

		list <MyProposition *> newPropositions;
		list <MyProposition *> oldPropositions;
		int nPropositions = myProblem.propositions.size();
		for (int j = 0; j < nPropositions; ++j){
			if (!myProblem.propositions[j].possibleEffective){
				continue;
			}
			if (isVisited(myProblem.propositions[j].firstVisitedLayer, lastLayer)){
				oldPropositions.push_back(&(myProblem.propositions[j]));
			}else if (myProblem.propositions[j].firstVisitedLayer == lastLayer + 1){
				newPropositions.push_back(&(myProblem.propositions[j]));
			}
		}


		cout << "Layer: " << numberOfLayers - 1 << ", operator: " << i << ", New Actions: " << newFoundedActions.size() << endl;

		/*************** Finding mutex relation between no-op and other actions ***************/

		list <MyAction *>::iterator actionIt, actionItEnd;
		list <MyProposition *>::iterator propositionIt, propositionItEnd;

		actionIt = newFoundedActions.begin();
		actionItEnd = newFoundedActions.end();
		for (; actionIt != actionItEnd; ++actionIt) {
			for (int j = 0; j < nPropositions; ++j){
				if (!isVisited(myProblem.propositions[j].firstVisitedLayer, lastLayer)){
					continue;
				}
				if ((*actionIt)->isPropositionStaticallyMutex(&(myProblem.propositions[j]))){
					continue;
				}
				if ((*actionIt)->checkDynamicPropositionMutex(lastLayer, &(myProblem.propositions[j]))){
					numberOfDynamicMutexesInLastLayer += 1;
					(*actionIt)->insertPropositionMutex(lastLayer, &(myProblem.propositions[j]));
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
				if (dynamicPropositionMutexIt->second == lastLayer - 1){
					if ((*actionIt)->checkDynamicPropositionMutex(lastLayer, dynamicPropositionMutexIt->first)){
						numberOfDynamicMutexesInLastLayer += 1;
						(*actionIt)->insertPropositionMutex(lastLayer, dynamicPropositionMutexIt->first);
					}
				}
			}
		}

		actionIt = oldFoundedActions.begin();
		actionItEnd = oldFoundedActions.end();
		propositionItEnd = oldPropositions.end();
		for (; actionIt != actionItEnd; ++actionIt) {
			propositionIt = oldPropositions.begin();
			for (;propositionIt != propositionItEnd; ++propositionIt){
				if ((*propositionIt)->firstVisitedLayer == lastLayer){
					if ((*actionIt)->isPropositionStaticallyMutex(*propositionIt)){
						continue;
					}
					if ((*actionIt)->checkDynamicPropositionMutex(lastLayer, *propositionIt)){
						numberOfDynamicMutexesInLastLayer += 1;
						(*actionIt)->insertPropositionMutex(lastLayer, *propositionIt);
					}
				}
			}
		}

		/*************** End of finding mutex relation between no-op and other actions ***************/



		/*************** Finding mutex relation propositions in new layer ***************/

		propositionIt = oldPropositions.begin();
		propositionItEnd = oldPropositions.end();
		for (;propositionIt != propositionItEnd; ++propositionIt){
			map <MyProposition *, int>::iterator propositionIt2, propositionItEnd2;
			propositionIt2 = (*propositionIt)->lastLayerMutexivity.begin();
			propositionItEnd2 = (*propositionIt)->lastLayerMutexivity.end();
			for (; propositionIt2 != propositionItEnd2; ++propositionIt2){
				if (propositionIt2->second == lastLayer && (*propositionIt)->checkMutex(lastLayer + 1, propositionIt2->first)){
					numberOfDynamicMutexesInLastLayer += 1;
					if ((*propositionIt)->originalLiteral->getStateID() < propositionIt2->first->originalLiteral->getStateID()){
						(*propositionIt)->insertMutex(lastLayer + 1, propositionIt2->first);
					}else{
						propositionIt2->first->insertMutex(lastLayer + 1, *propositionIt);
					}
				}
			}
		}

		propositionIt = newPropositions.begin();
		propositionItEnd = newPropositions.end();
		for (;propositionIt != propositionItEnd; ++propositionIt){
			list<MyProposition*>::iterator propositionIt2, propositionItEnd2;
			propositionIt2 = oldPropositions.begin();
			propositionItEnd2 = oldPropositions.end();
			for (;propositionIt2 != propositionItEnd2; ++propositionIt2){
				if ((*propositionIt)->checkMutex(lastLayer + 1, *propositionIt2)){
					numberOfDynamicMutexesInLastLayer += 1;
					if ((*propositionIt)->originalLiteral->getStateID() < (*propositionIt2)->originalLiteral->getStateID()){
						(*propositionIt)->insertMutex(lastLayer + 1, *propositionIt2);
					}else{
						(*propositionIt2)->insertMutex(lastLayer + 1, *propositionIt);
					}
				}
			}

			propositionIt2 = newPropositions.begin();
			propositionItEnd2 = newPropositions.end();
			for (; propositionIt2 != propositionItEnd2; ++propositionIt2){
				if ((*propositionIt)->originalLiteral->getStateID() <= (*propositionIt2)->originalLiteral->getStateID()){
					if ((*propositionIt)->checkMutex(lastLayer + 1, *propositionIt2)){
						numberOfDynamicMutexesInLastLayer += 1;
						(*propositionIt)->insertMutex(lastLayer + 1, *propositionIt2);
					}
				}
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



