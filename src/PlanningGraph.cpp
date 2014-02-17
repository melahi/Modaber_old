

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
	int nOperator = myProblem.actions.size();
	for (int i = 0; i < nOperator; ++i){
		int nActions = myProblem.actions[i].size();
		for (int j = 0; j < nActions; j++){
			myProblem.actions[i][j]->firstVisitedLayer = 0;
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
}

bool PlanningGraph::extendOneLayer(){

	if (numberOfLayers < 1){
		// First layer is not created yet, so we should first create it!
		createInitialLayer();
		return true;
	}

	cout << "constructing layer: " << numberOfLayers << " of my graph" << endl;

	bool canContinue = false;
	int nActions = instantiatedOp::howMany();
	vector <MyAction*> allActions(nActions);
	int nOperators = myProblem.actions.size();
	for (int i = 0; i < nOperators; ++i){
		int lng = myProblem.actions[i].size();
		for (int j = 0; j < lng; ++j){
			allActions[myProblem.actions[i][j]->id] = myProblem.actions[i][j];
		}
	}

	int lastLayer = (numberOfLayers - 1) * nActions;

	list <MyProposition *>::iterator prpIt, prpItEnd;


	for (int i = 0; i < nActions; ++i, ++lastLayer){
		if (!allActions[i]->possibleEffective){
			continue;
		}
		if (!isVisited(allActions[i]->firstVisitedLayer, lastLayer)){
			//If an action is not visited before, check its applicability, if it can be applied, then apply it!!!
			if (!allActions[i]->isApplicable(lastLayer)){
				continue;
			}
			allActions[i]->applyAction(lastLayer);
			canContinue =  true;
		}
		map <MyProposition *, int>::iterator prpIt2, prpIt2End;
		FOR_ITERATION(prpIt2, prpIt2End, allActions[i]->lastLayerPropositionMutexivity){
			if (prpIt2->second >= lastLayer - 1){
				allActions[i]->insertPropositionMutex(lastLayer - 1, prpIt2->first);
			}
		}
		FOR_ITERATION(prpIt, prpItEnd, allActions[i]->preconditionList){
			FOR_ITERATION(prpIt2, prpIt2End, (*prpIt)->lastLayerMutexivity){
				if (prpIt2->second >= lastLayer){
					allActions[i]->insertPropositionMutex(numeric_limits <int>::max(), prpIt2->first);
				}
			}
		}

		FOR_ITERATION(prpIt, prpItEnd, allActions[i]->addList){
			map <MyProposition *, int>::iterator prpIt2, prpIt2End;
			if ((*prpIt)->firstVisitedLayer == lastLayer + 1){
				FOR_ITERATION(prpIt2, prpIt2End, allActions[i]->lastLayerPropositionMutexivity){
					bool addedBySameAction = false;
					list <MyProposition *>::iterator prpIt4, prpIt4End;
					FOR_ITERATION(prpIt4, prpIt4End, allActions[i]->addList){
						if ((*prpIt4)->originalLiteral->getStateID() == prpIt2->first->originalLiteral->getStateID()){
							addedBySameAction = true;
							break;
						}
					}
					if (!addedBySameAction){
						(*prpIt)->insertMutex(numeric_limits <int>::max(), prpIt2->first);
					}
				}
				list <MyProposition*>::iterator prpIt3, prpIt3End;
				FOR_ITERATION(prpIt3, prpIt3End, allActions[i]->deleteList){
					(*prpIt)->insertMutex(numeric_limits <int>::max(), *prpIt3);
				}

			}
			FOR_ITERATION(prpIt2, prpIt2End, (*prpIt)->lastLayerMutexivity){
				if (prpIt2->second >= lastLayer + 1){
					bool addedBySameAction = false;
					list <MyProposition *>::iterator prpIt4, prpIt4End;
					FOR_ITERATION(prpIt4, prpIt4End, allActions[i]->addList){
						if ((*prpIt4)->originalLiteral->getStateID() == prpIt2->first->originalLiteral->getStateID()){
							addedBySameAction = true;
							break;
						}
					}
					if (addedBySameAction || allActions[i]->isPropositionMutex(lastLayer, prpIt2->first) == false){
						(*prpIt)->insertMutex(lastLayer, prpIt2->first);
						canContinue = true;
					}
				}
			}

		}
	}

	numberOfLayers++;

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



