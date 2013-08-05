

#include "OldNumericalPlanningGraph.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"

using namespace VAL;
using namespace Inst;

void NumericalPlanningGraph::createInitialLayer(){
	if (numberOfLayers > 0){
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
			propositions[lit2->getStateID()].firstVisitedLayer = 0;
		}
	}

	numberOfLayers = 1;
	numberOfMutexesInLastLayer = 0;

}

bool NumericalPlanningGraph::extendOneLayer(){
	if (numberOfLayers < 1){
		// First layer is not created yet, so we should first create it!
		createInitialLayer();
		return true;
	}

	bool canContinue = false;

	//If there is any action which is not executed before, check its applicability, if it can be applied, then apply it!!!
	int nPropositions = propositions.size();
	int nAction = actions.size();
	for (int i = 0; i < nAction; i++){
		if (actions[i].firstVisitedLayer == -1 && actions[i].permanentMutex.find(i) == actions[i].permanentMutex.end()){
			if (isApplicable(i, numberOfLayers - 1)){
				/* We should check that no precondition of the action be
				 * mutex with any other precondition of it
				 */
				if (!actions[i].checkMutex(numberOfLayers - 1, &actions[i])){
					applyAction(i, numberOfLayers - 1);
					// If a new action can be applied it means that our graph doesn't level off and we should continue for another layer
					canContinue = true;
				}
			}
		}
	}

	int tempNumberOfMutex = numberOfMutexesInLastLayer;
	numberOfMutexesInLastLayer = 0;


	//Find proposition (no-op) and action mutex for the layer
	for (int i = 0; i < nAction; i++){
		if (actions[i].firstVisitedLayer == -1){
			continue;
		}
		for (int j = 0; j < nPropositions; j++){
			if (propositions[j].firstVisitedLayer == -1 || propositions[j].firstVisitedLayer >= numberOfLayers){
				continue;
			}
			if (actions[i].checkPropositionMutex(numberOfLayers - 1, &propositions[j])){
				numberOfMutexesInLastLayer += 2;
				actions[i].insertPropositionMutex(numberOfLayers - 1, j);
			}
		}
	}



	//Find action mutex for new layer
	for (int i = 0; i < nAction; i++){
		if (actions[i].firstVisitedLayer == -1){
			continue;
		}
		for (int j = i + 1; j < nAction; j++){
			if (actions[j].firstVisitedLayer == -1){
				continue;
			}
			if (actions[i].checkMutex(numberOfLayers - 1, &actions[j])){
				numberOfMutexesInLastLayer += 2;
				actions[i].insertMutex(numberOfLayers - 1, j);
				actions[j].insertMutex(numberOfLayers - 1, i);
			}
		}
	}

	//Find proposition mutex for new layer
	for (int i = 0; i < nPropositions; i++){
		if (propositions[i].firstVisitedLayer == -1){
			continue;
		}
		for (int j = i + 1; j < nPropositions; j++){
			if (propositions[j].firstVisitedLayer == -1){
				continue;
			}
			if (propositions[i].checkMutex(numberOfLayers, &propositions[j])){
				numberOfMutexesInLastLayer += 2;
				propositions[i].insertMutex(numberOfLayers, j);
				propositions[j].insertMutex(numberOfLayers, i);

			}
		}
	}


	numberOfLayers++;


	if (numberOfMutexesInLastLayer < tempNumberOfMutex){
		canContinue = true;
	}


	return canContinue;
}

void NumericalPlanningGraph::applyAction (int actionId, int layerNumber){

	actions[actionId].firstVisitedLayer = layerNumber;

	instantiatedOp *op = instantiatedOp::getInstOp(actionId);
	FastEnvironment *env = op->getEnv();
	addSimpleEffectList(op->forOp()->effects->add_effects, env, layerNumber + 1, actionId);
}

void NumericalPlanningGraph::addSimpleEffectList (const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber, int actionId){
	pc_list<simple_effect*>::const_iterator it = simpleEffectList.begin();
	pc_list<simple_effect*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);

		propositions[lit2->getStateID()].provider.push_back(&actions[actionId]);

		if (lit2->getStateID() != -1 && propositions[lit2->getStateID()].firstVisitedLayer == -1){
			propositions[lit2->getStateID()].firstVisitedLayer = layerNumber;
		}
	}
}



bool NumericalPlanningGraph::isApplicable (int actionId, int layerNumber) {
	actions[actionId].precondition.clear();
	instantiatedOp *op = instantiatedOp::getInstOp(actionId);
	FastEnvironment *env = op->getEnv();
	return (isPreconditionSatisfied(op->forOp()->precondition, env, layerNumber, actionId));
}



bool NumericalPlanningGraph::isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber, int actionId){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(precondition);
	if (simple){
		if (simple->getPolarity() == E_NEG){
			return true;
		}
		Literal lit(simple->getProp(), env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);

		actions[actionId].precondition.push_back(&propositions[lit2->getStateID()]);
		if (lit2->getStateID() == -1){
			return true;
		}

		if (propositions[lit2->getStateID()].firstVisitedLayer != -1 && propositions[lit2->getStateID()].firstVisitedLayer <= layerNumber){
			return true;
		}
		return false;
	}
	const comparison *comp = dynamic_cast<const comparison*> (precondition);
	if (comp){
		return true;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(precondition);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			if (!isPreconditionSatisfied(*it, env, layerNumber, actionId) ){
				return false;
			}
		}
		return true;
	}
	CANT_HANDLE("can't evaluate some precondition");
	return true;
}


NumericalPlanningGraph::NumericalPlanningGraph(int nPropositions, int nActions, MyAnalyzer *myAnalyzer) {
	actions.resize(nActions);
	for (int i = 0; i < nActions; i++){
		actions[i].computePermanentMutex(i, myAnalyzer);
	}

	propositions.resize(nPropositions);
	for (int i = 0; i < nPropositions; i++){
		propositions[i].propositionId = i;
	}

	numberOfLayers = 0;

	createInitialLayer();
	bool canContinue = true;
	while (canContinue){
		cout << numberOfLayers << endl;
		canContinue = extendOneLayer();
	}
	print(cout);
}

void NumericalPlanningGraph::print(ostream &sout){
	int nActions = actions.size();
	int nPropositions = propositions.size();

	vector <Literal*> nonStaticLiterals;
	nonStaticLiterals.resize(nPropositions);

	for (LiteralStore::iterator it = instantiatedOp::literalsBegin(); it != instantiatedOp::literalsEnd(); ++it){
		if ((*it)->getStateID() != -1){
			nonStaticLiterals[(*it)->getStateID()] = *it;
		}
	}


	sout << "Number of Layers: " << numberOfLayers << endl;
	for (int i = 0; i < numberOfLayers; i++){

		//Print propositions
		for (int j = 0; j < nPropositions; j++){
			if (propositions[j].firstVisitedLayer == i){
				nonStaticLiterals[j]->write(sout);
				sout << ';';
			}
		}
		sout << endl;

		//Print propositions mutexes
		for (int j = 0; j < nPropositions; j++){
			if (propositions[j].firstVisitedLayer == -1 || propositions[j].firstVisitedLayer > i){
				continue;
			}
			for (int k = 0; k < nPropositions; k++){
				if (propositions[k].firstVisitedLayer != -1 && propositions[k].firstVisitedLayer <= i){
					if (propositions[j].isMutex(i, &propositions[k])){
						sout << '(';
						nonStaticLiterals[j]->write(sout);
						sout << ',';
						nonStaticLiterals[k]->write(sout);
						sout << ");";
					}
				}
			}
		}
		sout << endl;

		//Print actions
		for (int j = 0; j < nActions; j++){
			if (actions[j].firstVisitedLayer == i){
				instantiatedOp::getInstOp(j)->write(sout);
				sout << ';';
			}
		}
		sout << endl;

		//Print actions mutexes
		for (int j = 0; j < nActions; j++){
			if (actions[j].firstVisitedLayer == -1 || actions[j].firstVisitedLayer > i){
				continue;
			}
			for (int k = 0; k < nActions; k++){
				if (actions[k].firstVisitedLayer != -1 && actions[k].firstVisitedLayer <= i){
					if (actions[j].isMutex(i, &actions[k])){
						sout << '(';
						instantiatedOp::getInstOp(j)->write(sout);
						sout << ',';
						instantiatedOp::getInstOp(k)->write(sout);
						sout << ");";
					}
				}
			}
		}
		sout << endl;
	}
}

NumericalPlanningGraph::~NumericalPlanningGraph() {
	// TODO Auto-generated destructor stub
}

