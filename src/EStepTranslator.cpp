
#include "EStepTranslator.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include "Utilities.h"
#include "MyProblem.h"
#include "LiftedCVC4Problem.h"
#include "EStepModaber.h"

#include "VALfiles/FastEnvironment.h"
#include <list>

using namespace std;
using namespace Inst;
using namespace VAL;


namespace mdbr {




void EStepTranslator::prepareGoals() {
	addGoals(translatedLength - 1);
}

void EStepTranslator::prepare (int length, MyAction *metricFunction){

	if (translatedLength > length){
		CANT_HANDLE("prepare function is called with the smaller number of length than it is translated");
		return;
	}

	for (; translatedLength < length; translatedLength++){
		addActions(translatedLength - 1);
		addExplanatoryAxioms(translatedLength - 1);
		addAtomMutex(translatedLength - 1);
	}

	prepareGoals();
	addMetricFunction(translatedLength - 1, metricFunction);
}


//Translate initial state of planning problem to SMT problem
void EStepTranslator::addInitialState(){
	vector <bool> initialState;
	initialState.resize(instantiatedOp::howManyNonStaticLiterals(), false);

	pc_list<simple_effect*>::const_iterator it = current_analysis->the_problem->initial_state->add_effects.begin();
	pc_list<simple_effect*>::const_iterator itEnd = current_analysis->the_problem->initial_state->add_effects.end();
	FastEnvironment env(0);

	for (; it != itEnd; it++){
		Literal lit ((*it)->prop, &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() != -1){
			initialState[lit2->getStateID()] = true;
		}
	}

	for (unsigned int i = 0; i < initialState.size(); i++) {
		if (myProblem.propositions[i].possibleEffective){
			solver.addProposition(i, 0, 0, initialState[i]);
			solver.endClause();
		}
	}

	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		if (myProblem.variables[i].domain.size() < 3){
			continue;
		}
		map <double, vector <int> >::iterator domainIt, domainItEnd;
		FOR_ITERATION(domainIt, domainItEnd, myProblem.variables[i].domain){
			bool polarity = false;
			if (domainIt->first == myProblem.initialValue[myProblem.variables[i].originalPNE->getGlobalID()]){
				polarity = true;
			}
			solver.addValue(domainIt, upperBound, 0, 0, polarity);
			solver.endClause();
			solver.addValue(domainIt, lowerBound, 0, 0, polarity);
			solver.endClause();
		}
	}
}

//Add goals to the smt problem
void EStepTranslator::addGoals (int significantTimePoint){
	FastEnvironment env(0);
	addGoal(current_analysis->the_problem->the_goal, &env, significantTimePoint);
}


void EStepTranslator::addMetricFunction (int significantTimePoint, MyAction *metricFunction){
	if (metricFunction){
		addUnacceptablePreconditionBoundaries(significantTimePoint, metricFunction);
	}
}

//Insert actions' conditions for the specified time point in smt problem
void EStepTranslator::addActions (int significantTimePoint){
	int nOperators = myProblem.actions.size();
	for (int i = 0; i < nOperators; ++i){
		int nActions = myProblem.actions[i].size();
		for (int j = 0; j < nActions; ++j){
			addPropositionList(true, significantTimePoint, myProblem.actions[i][j]->addList.begin(), myProblem.actions[i][j]->addList.end(), myProblem.actions[i][j]->id, myProblem.actions[i][j]->id + 1);
			addPropositionList(false, significantTimePoint, myProblem.actions[i][j]->deleteList.begin(), myProblem.actions[i][j]->deleteList.end(), myProblem.actions[i][j]->id, myProblem.actions[i][j]->id + 1);
			addPropositionList(true, significantTimePoint, myProblem.actions[i][j]->preconditionList.begin(), myProblem.actions[i][j]->preconditionList.end(), myProblem.actions[i][j]->id, myProblem.actions[i][j]->id);
			addUnacceptablePreconditionBoundaries(significantTimePoint, myProblem.actions[i][j]);
			addAssignmentBoundaries(significantTimePoint, myProblem.actions[i][j]);
		}
	}
}


//Insert Explanatory Axioms which is needed for SMT problem
void EStepTranslator::addExplanatoryAxioms (int significantTimePoint){

	vector <MyAction* >::iterator aIt, aItEnd;

	int nProposition = myProblem.propositions.size();

	for (int i = 0; i < nProposition; i++){

		FOR_ITERATION(aIt, aItEnd, myProblem.propositions[i].adder_groundAction){
			solver.addAction((*aIt)->id, significantTimePoint, true);
			solver.addProposition(i, (*aIt)->id, significantTimePoint, true);
			solver.addProposition(i, (*aIt)->id + 1, significantTimePoint, false);
			solver.endClause();
			solver.addAction((*aIt)->id, significantTimePoint, true);
			solver.addProposition(i, (*aIt)->id, significantTimePoint, false);
			solver.addProposition(i, (*aIt)->id + 1, significantTimePoint, true);
			solver.endClause();
		}


		FOR_ITERATION(aIt, aItEnd, myProblem.propositions[i].deleter_groundAction){
			solver.addAction((*aIt)->id, significantTimePoint, true);
			solver.addProposition(i, (*aIt)->id, significantTimePoint, true);
			solver.addProposition(i, (*aIt)->id + 1, significantTimePoint, false);
			solver.endClause();
			solver.addAction((*aIt)->id, significantTimePoint, true);
			solver.addProposition(i, (*aIt)->id, significantTimePoint, false);
			solver.addProposition(i, (*aIt)->id + 1, significantTimePoint, true);
			solver.endClause();
		}
	}


	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){

		FOR_ITERATION(aIt, aItEnd, myProblem.variables[i].modifier_groundAction){
			map <double, vector <int> >::iterator domainIt, domainItEnd;
			FOR_ITERATION(domainIt, domainItEnd, myProblem.variables[i].domain){

				//Upper bound
				solver.addAction((*aIt)->id, significantTimePoint, true);
				solver.addValue(domainIt, upperBound, (*aIt)->id, significantTimePoint, true);
				solver.addValue(domainIt, upperBound, (*aIt)->id + 1, significantTimePoint, false);
				solver.endClause();
				solver.addAction((*aIt)->id, significantTimePoint, true);
				solver.addValue(domainIt, upperBound, (*aIt)->id, significantTimePoint, false);
				solver.addValue(domainIt, upperBound, (*aIt)->id + 1, significantTimePoint, true);
				solver.endClause();

				//Lower bound
				solver.addAction((*aIt)->id, significantTimePoint, true);
				solver.addValue(domainIt, lowerBound, (*aIt)->id, significantTimePoint, true);
				solver.addValue(domainIt, lowerBound, (*aIt)->id + 1, significantTimePoint, false);
				solver.endClause();
				solver.addAction((*aIt)->id, significantTimePoint, true);
				solver.addValue(domainIt, lowerBound, (*aIt)->id, significantTimePoint, false);
				solver.addValue(domainIt, lowerBound, (*aIt)->id + 1, significantTimePoint, true);
				solver.endClause();

			}
		}
	}
}

void EStepTranslator::addAtomMutex(int significantTimePoint){
	int nProposition = myProblem.propositions.size();
	int nActions = instantiatedOp::howMany();
	for (int i = 0; i < nProposition; ++i){
		for (int j = 0; j < i; ++j){
			int lastId1, lastId2;
			lastId1 = -10;
			lastId2 = -10;
			int lastLayer = significantTimePoint * nActions;
			for (int k = 0; k < nActions; ++k, ++lastLayer){
				if ((!isVisited(myProblem.propositions[i].firstVisitedLayer, lastLayer)) || (!isVisited(myProblem.propositions[j].firstVisitedLayer, lastLayer))){
					continue;
				}
				if (myProblem.propositions[i].isMutex(lastLayer, &myProblem.propositions[j]) == false){
					break;
				}
				if (myProblem.propositions[i].ids[k] == lastId1 && myProblem.propositions[j].ids[k] == lastId2){
					continue;
				}
				solver.addProposition(i, k, significantTimePoint, false);
				solver.addProposition(j, k, significantTimePoint, false);
				solver.endClause();
				lastId1 = myProblem.propositions[i].ids[k];
				lastId2 = myProblem.propositions[j].ids[k];
			}
		}
	}


	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		if (myProblem.variables[i].domain.size() == 0){
			continue;
		}
		map <double, vector <int> >::iterator domainIt, domainIt2, domainItEnd;
		int lastID = myProblem.variables[i].domain.begin()->second[0];
		domainItEnd = myProblem.variables[i].domain.end();
		for (int j = 1; lastID != -1 && j <= nActions; ++j){
			domainIt = myProblem.variables[i].domain.begin();
			if (j != nActions){
				if (lastID == domainIt->second[j]){
					continue;
				}
				lastID = domainIt->second[j];
			}
			for (; domainIt != domainItEnd; ++domainIt){
				domainIt2 = domainIt;
				++domainIt2;
				for (; domainIt2 != domainItEnd; ++domainIt2){
					solver.addValue(domainIt, upperBound, j, significantTimePoint, false);
					solver.addValue(domainIt2, upperBound, j, significantTimePoint, false);
					solver.endClause();
					solver.addValue(domainIt, lowerBound, j, significantTimePoint, false);
					solver.addValue(domainIt2, lowerBound, j, significantTimePoint, false);
					solver.endClause();
				}
			}

			domainIt = myProblem.variables[i].domain.begin();
			for (; domainIt != domainItEnd; ++domainIt){
				solver.addValue(domainIt, upperBound, j, significantTimePoint, true);
			}
			solver.endClause();

			domainIt = myProblem.variables[i].domain.begin();
			for (; domainIt != domainItEnd; ++domainIt){
				solver.addValue(domainIt, lowerBound, j, significantTimePoint, true);
			}
			solver.endClause();

		}
	}

}

void EStepTranslator::addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyAction *action /* = NULL */){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		Literal lit (simple->getProp(), env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (action){
			solver.addAction(action->id, significantTimePoint, false);
			solver.addProposition(lit2->getStateID(), action->id, significantTimePoint, (simple->getPolarity() == E_POS));
		}else{
			solver.addProposition(lit2->getStateID(), 0, significantTimePoint, (simple->getPolarity() == E_POS));
		}
		solver.endClause();
		return;
	}
	const comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			addGoal(*it, env, significantTimePoint, action);
		}
		return;
	}
	const preference *thePreference = dynamic_cast <const preference *> (gl);
	if (thePreference){
		return;
	}

	CANT_HANDLE("can't translate some GOAL");
}


void EStepTranslator::addPropositionList (bool polarity, int significantTimePoint, list <MyProposition*>::const_iterator it, list <MyProposition*>::const_iterator itEnd, int actionId, int effectiveId){
	for (; it != itEnd; ++it){
		solver.addAction(actionId, significantTimePoint, false);
		solver.addProposition((*it)->originalLiteral->getStateID(), effectiveId, significantTimePoint, polarity);
		solver.endClause();
	}
}

//void EStepTranslator::findGoalList (const goal *gl, list <const simple_goal *> &returningList){
//	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
//	if (simple){
//		returningList.push_back(simple);
//		return;
//	}
//	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
//	if (conjunctive){
//		const goal_list *goalList = conjunctive->getGoals();
//		goal_list::const_iterator it = goalList->begin();
//		goal_list::const_iterator itEnd = goalList->end();
//		for (; it != itEnd; it++){
//			findGoalList(*it, returningList);
//		}
//		return;
//	}
//	CANT_HANDLE("can't handle some goal in findingGoalList");
//}


void EStepTranslator::addUnacceptablePreconditionBoundaries (int significantTimePoint, MyAction *action){
	int lng = action->unacceptablePreconditionBoundaries.size();
	int actionId;
	for (int i = 0; i < lng; ++i){
		vector <MyBound>::iterator it, itEnd;
		if ((unsigned int) action->id != -10){
			solver.addAction(action->id, significantTimePoint, false);
			actionId = action->id;
		}else{
			actionId = 0;
		}
		FOR_ITERATION(it, itEnd, action->unacceptablePreconditionBoundaries[i]){
			solver.addValue(it->member, it->kind, actionId, significantTimePoint, false);
		}
		solver.endClause();
	}
}

void EStepTranslator::addAssignmentBoundaries (int significantTimePoint, MyAction *action){
	int lng = action->assignmentBoundaries.size();
	for (int i = 0; i < lng; ++i){
		vector <MyBound>::iterator it, itEnd;
		solver.addAction(action->id, significantTimePoint, false);
		FOR_ITERATION(it, itEnd, action->assignmentBoundaries[i].first){
			solver.addValue(it->member, it->kind, action->id, significantTimePoint, false);
		}
		solver.addValue(action->assignmentBoundaries[i].second.member, action->assignmentBoundaries[i].second.kind, action->id + 1, significantTimePoint, true);
		solver.endClause();
	}
}



bool EStepTranslator::solve(){
	//try to solve the problem
	return solver.solving();
	return true;
}


void EStepTranslator::getSolution(vector <pair <operator_ *, FastEnvironment> > &solution){
	solution.clear();
	int nActions = instantiatedOp::howMany();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nActions; ++j){
			if (solver.isTrueAction(j, i)){
				operator_ *theOperator = const_cast <operator_ *> ((*(instantiatedOp::from(j)))->forOp());
				pair <operator_ *, FastEnvironment> pr (theOperator, *(*(instantiatedOp::from(j)))->getEnv());
				solution.push_back(pr);
			}
		}
	}
}


void EStepTranslator::extractSolution (ostream &sout){
	int nActions = instantiatedOp::howMany();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nActions; ++j){
			if (solver.isTrueAction(j, i)){
				(*instantiatedOp::from(j))->write(sout);
				sout << endl;
			}
		}
	}
}

} /* namespace mdbr */
