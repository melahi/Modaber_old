/*
 * SATLiftedTranslator.cpp
 *
 *  Created on: Sep 28, 2013
 *      Author: sadra
 */

#include "SATLiftedTranslator.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include "Utilities.h"
#include "MyProblem.h"
#include "LiftedCVC4Problem.h"

#include "VALfiles/FastEnvironment.h"
#include <list>

using namespace std;
using namespace Inst;
using namespace VAL;


namespace mdbr {




void SATLiftedTranslator::prepareGoals() {
	addGoals(translatedLength - 1);
}

void SATLiftedTranslator::prepare (int length, MyPartialAction *metricFunction){

	if (translatedLength > length){
		CANT_HANDLE("prepare function is called with the smaller number of length than it is translated");
		return;
	}

	if (translatedLength == length){
		prepareGoals();
		addMetricFunction(translatedLength - 1, metricFunction);
		return;
	}

	for (; translatedLength < length; translatedLength++){
		addPartialActions(translatedLength - 1);
		addCompletingAction(translatedLength - 1);
		addExplanatoryAxioms(translatedLength - 1);
		addAtomMutex(translatedLength - 1);
	}

	prepareGoals();
	addMetricFunction(translatedLength - 1, metricFunction);
}


//Translate initial state of planning problem to SMT problem
void SATLiftedTranslator::addInitialState(){
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
void SATLiftedTranslator::addGoals (int significantTimePoint){
	FastEnvironment env(0);
	addGoal(current_analysis->the_problem->the_goal, &env, significantTimePoint);
}


void SATLiftedTranslator::addMetricFunction (int significantTimePoint, MyPartialAction *metricFunction){
	if (metricFunction){
		addUnacceptablePreconditionBoundaries(significantTimePoint, metricFunction);
	}
}

//Insert actions' conditions for the specified time point in smt problem
void SATLiftedTranslator::addPartialActions (int significantTimePoint){
	list <MyPartialAction>::iterator it, itEnd;
	it = myProblem.partialAction.begin();
	itEnd = myProblem.partialAction.end();
	for (; it != itEnd; ++it){

		MyOperator *theOperator = it->partialOperator->op;
		if (it->partialOperator->argument.size() != 0){
			set <const VAL::symbol*>::iterator it1, it1End;

			FOR_ITERATION(it1, it1End, it->partialOperator->argument){
				solver.addAction(it->id, significantTimePoint, false);
				solver.addUnification(theOperator->unificationId[theOperator->argument[*it1]][(*(it->env))[*it1]->getName()], significantTimePoint, true);
				solver.endClause();
			}
		}


		if (it->isValid == false){
			solver.addAction(it->id, significantTimePoint, false);
			solver.endClause();
			continue;
		}

		addSimpleEffectList(E_POS, it->addEffect, significantTimePoint, &(*it));
		addSimpleEffectList(E_NEG, it->deleteEffect, significantTimePoint, &(*it));
		addGoalList(it->precondition, significantTimePoint, &(*it));
		addUnacceptablePreconditionBoundaries(significantTimePoint, &(*it));
		addAssignmentBoundaries(significantTimePoint, &(*it));
	}
}


//Insert Explanatory Axioms which is needed for SMT problem
void SATLiftedTranslator::addExplanatoryAxioms (int significantTimePoint){

	list <MyPartialAction* >::iterator pAIt, pAItEnd;

	int nProposition = myProblem.propositions.size();
	int nOperators = myProblem.operators.size();

	for (int i = 0; i < nProposition; i++){

		vector < list<MyPartialAction *> > adder(nOperators);
		pAIt = myProblem.propositions[i].adder.begin();
		pAItEnd = myProblem.propositions[i].adder.end();
		for (; pAIt != pAItEnd; ++pAIt){
			adder[(*pAIt)->partialOperator->op->id].push_back(*pAIt);
		}

		vector < list<MyPartialAction *> > deleter(nOperators);
		pAIt = myProblem.propositions[i].deleter.begin();
		pAItEnd = myProblem.propositions[i].deleter.end();
		for (; pAIt != pAItEnd; ++pAIt){
			deleter[(*pAIt)->partialOperator->op->id].push_back(*pAIt);
		}


		for (int j = 0; j < nOperators; ++j){
			if (adder[j].size() || deleter[j].size() || (j == 0 && myProblem.propositions[i].adder.size() + myProblem.propositions[i].deleter.size() == 0)){

				//Deleter actions
				solver.addProposition(i, j, significantTimePoint, false);
				solver.addProposition(i, j + 1, significantTimePoint, true);
				pAIt = deleter[j].begin();
				pAItEnd = deleter[j].end();
				for (; pAIt != pAItEnd; ++pAIt){
					solver.addAction((*pAIt)->id, significantTimePoint, true);
				}
				solver.endClause();

				//Adder Actions
				solver.addProposition(i, j, significantTimePoint, true);
				solver.addProposition(i, j + 1, significantTimePoint, false);
				pAIt = adder[j].begin();
				pAItEnd = adder[j].end();
				for (; pAIt != pAItEnd; ++pAIt){
					solver.addAction((*pAIt)->id, significantTimePoint, true);
				}
				solver.endClause();
			}
		}
	}


	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		vector <list <MyPartialAction *> > modifier;
		modifier.resize(nOperators);
		FOR_ITERATION(pAIt, pAItEnd, myProblem.variables[i].modifier){
			modifier[(*pAIt)->partialOperator->op->id].push_back(*pAIt);
		}

		for (int j = 0; j < nOperators; ++j){
			if (!modifier[j].size()){
				continue;
			}
			map <double, vector <int> >::iterator domainIt, domainItEnd;
			FOR_ITERATION(domainIt, domainItEnd, myProblem.variables[i].domain){
				//FOR UPPER BOUND
				solver.addValue(domainIt, upperBound, j, significantTimePoint, true);
				solver.addValue(domainIt, upperBound, j + 1, significantTimePoint, false);
				FOR_ITERATION(pAIt, pAItEnd, modifier[j]){
					solver.addAction((*pAIt)->id, significantTimePoint, true);
				}
				solver.endClause();

				//FOR LOWER BOUND
				solver.addValue(domainIt, lowerBound, j, significantTimePoint, true);
				solver.addValue(domainIt, lowerBound, j + 1, significantTimePoint, false);
				FOR_ITERATION(pAIt, pAItEnd, modifier[j]){
					solver.addAction((*pAIt)->id, significantTimePoint, true);
				}
				solver.endClause();

			}
		}
	}

}

void SATLiftedTranslator::addCompletingAction (int significantTimePoint){
	int nOperators = myProblem.operators.size();
	for (int i = 0; i < nOperators; ++i){
		int nArguments = myProblem.operators[i]->argument.size();
		for (int j = 0; j < nArguments; ++j){

			map <string, int>::iterator it1, it2, itEnd1, itEnd2;

			//At most on unification for each argument should be true
			FOR_ITERATION(it1, itEnd1, myProblem.operators[i]->unificationId[j]) {
				it2 = myProblem.operators[i]->unificationId[j].begin();
				for (; it2 != it1; ++it2){
					solver.addUnification(it1->second, significantTimePoint, false);
					solver.addUnification(it2->second, significantTimePoint, false);
					solver.endClause();
				}
			}
		}
	}



	for (int i = 0; i < nOperators; ++i){
		int nPartialOperators = myProblem.operators[i]->partialOperator.size();
		for (int j = 0; j < nPartialOperators; ++j){
			if (nPartialOperators == 1){
				continue;
			}
			int nPartialActions = myProblem.operators[i]->partialOperator[j]->child.size();
			int nextPartialOperator = (j + 1) % nPartialOperators;
			int nNextPartialActions = myProblem.operators[i]->partialOperator[nextPartialOperator]->child.size();
			for (int k = 0; k < nPartialActions; k++){
				solver.addAction(myProblem.operators[i]->partialOperator[j]->child[k]->id, significantTimePoint, false);
				for (int l = 0; l < nNextPartialActions; ++l){
					solver.addAction(myProblem.operators[i]->partialOperator[nextPartialOperator]->child[l]->id, significantTimePoint, true);
				}
				solver.endClause();
			}
		}
	}
}

void SATLiftedTranslator::addAtomMutex(int significantTimePoint){
	int nProposition = myProblem.propositions.size();
	int nOperators = myProblem.operators.size();
	for (int i = 0; i < nProposition; ++i){
		for (int j = 0; j < i; ++j){
			int lastId1, lastId2;
			lastId1 = -10;
			lastId2 = -10;
			for (int k = 0; k < nOperators; ++k){
				int layerNumber = (significantTimePoint * nOperators) + k;
				if (myProblem.propositions[i].isMutex (layerNumber, &(myProblem.propositions[j]))){
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
	}


	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		if (myProblem.variables[i].domain.size() == 0){
			continue;
		}
		map <double, vector <int> >::iterator domainIt, domainIt2, domainItEnd;
		int lastID = myProblem.variables[i].domain.begin()->second[0];
		domainItEnd = myProblem.variables[i].domain.end();
		for (int j = 1; lastID != -1 && j <= nOperators; ++j){
			domainIt = myProblem.variables[i].domain.begin();
			if (j != nOperators){
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

void SATLiftedTranslator::addSimpleEffectList (polarity plrty, const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction){
	list <MyProposition*>::const_iterator it = simpleEffectList.begin();
	list <MyProposition*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		solver.addAction(partialAction->id, significantTimePoint, false);
		solver.addProposition((*it)->originalLiteral->getStateID(), partialAction->partialOperator->op->id + 1, significantTimePoint, (plrty == E_POS));
		solver.endClause();
	}
}

void SATLiftedTranslator::addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction /* = NULL */){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		Literal lit (simple->getProp(), env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (partialAction){
			solver.addAction(partialAction->id, significantTimePoint, false);
			solver.addProposition(lit2->getStateID(), partialAction->partialOperator->op->id, significantTimePoint, (simple->getPolarity() == E_POS));
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
			addGoal(*it, env, significantTimePoint, partialAction);
		}
		return;
	}
	const preference *thePreference = dynamic_cast <const preference *> (gl);
	if (thePreference){
		return;
	}

	CANT_HANDLE("can't translate some GOAL");
}


void SATLiftedTranslator::addGoalList (const list <MyProposition *> &preconditionList, int significantTimePoint, MyPartialAction *partialAction){
	list <MyProposition*>::const_iterator it = preconditionList.begin();
	list <MyProposition*>::const_iterator itEnd = preconditionList.end();
	for (; it != itEnd; ++it){
		solver.addAction(partialAction->id, significantTimePoint, false);
		solver.addProposition((*it)->originalLiteral->getStateID(), partialAction->partialOperator->op->id, significantTimePoint, true);
		solver.endClause();
	}
}

//void SATLiftedTranslator::findGoalList (const goal *gl, list <const simple_goal *> &returningList){
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


void SATLiftedTranslator::addUnacceptablePreconditionBoundaries (int significantTimePoint, MyPartialAction *partialAction){
	int lng = partialAction->unacceptablePreconditionBoundaries.size();
	int operatorId;
	for (int i = 0; i < lng; ++i){
		vector <MyBound>::iterator it, itEnd;
		if ((unsigned int) partialAction->id != myProblem.partialAction.size()){
			solver.addAction(partialAction->id, significantTimePoint, false);
			operatorId = partialAction->partialOperator->op->id;
		}else{
			operatorId = 0;
		}
		FOR_ITERATION(it, itEnd, partialAction->unacceptablePreconditionBoundaries[i]){
			solver.addValue(it->member, it->kind, operatorId, significantTimePoint, false);
		}
		solver.endClause();
	}
}

void SATLiftedTranslator::addAssignmentBoundaries (int significantTimePoint, MyPartialAction *partialAction){
	int lng = partialAction->assignmentBoundaries.size();
	for (int i = 0; i < lng; ++i){
		vector <MyBound>::iterator it, itEnd;
		solver.addAction(partialAction->id, significantTimePoint, false);
		FOR_ITERATION(it, itEnd, partialAction->assignmentBoundaries[i].first){
			solver.addValue(it->member, it->kind, partialAction->partialOperator->op->id, significantTimePoint, false);
		}
		solver.addValue(partialAction->assignmentBoundaries[i].second.member, partialAction->assignmentBoundaries[i].second.kind, partialAction->partialOperator->op->id + 1, significantTimePoint, true);
		solver.endClause();
	}
}



bool SATLiftedTranslator::solve(){


	//try to solve the problem
	return solver.solving();
	return true;
}


void SATLiftedTranslator::getSolution(vector <pair <operator_ *, FastEnvironment> > &solution){
	solution.clear();
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nOperator; ++j){
			int nPartialOperator = myProblem.operators[j]->partialOperator.size();
			FastEnvironment env(myProblem.operators[j]->originalOperator->parameters->size());
			for (int k= 0; k < nPartialOperator; ++k){
				int nPartialActions = myProblem.operators[j]->partialOperator[k]->child.size();
				int l;
				for (l = 0; l < nPartialActions; ++l){
					if (solver.isTrueAction(myProblem.operators[j]->partialOperator[k]->child[l]->id, i)){
//						sout << ";;Partial action: "; myProblem.operators[j]->partialOperator[k]->child[l]->write(sout); sout << endl;
						break;
					}
				}
				if (l == nPartialActions){
					if (k != 0){
						CANT_HANDLE("Oops, some actions was incompletely done");
					}
					break;
				}else{
					set <const VAL::symbol *>::const_iterator it, itEnd;
					initializeIterator(it, itEnd, myProblem.operators[j]->partialOperator[k]->argument);
					for (; it != itEnd; ++it){
						env[*it] = (*(myProblem.operators[j]->partialOperator[k]->child[l]->env))[*it];
					}
				}
				if (k == nPartialOperator - 1){
					var_symbol_list::iterator paramIt, paramItEnd;
					initializeIterator(paramIt, paramItEnd, (*(myProblem.operators[j]->originalOperator->parameters)));
					for (; paramIt != paramItEnd; ++paramIt){
						if ((env[*paramIt]) == NULL){
							//It means some parameters of action is not bind, so I assume, I can bind it to any object!!!
							env[*paramIt] = (*(myProblem.actions[j][0]->valAction->getEnv()))[*paramIt];
						}
					}
					solution.push_back(pair <operator_ *, FastEnvironment> (myProblem.operators[j]->originalOperator, env));
				}
			}
		}
	}
}


void SATLiftedTranslator::extractSolution (ostream &sout){
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nOperator; ++j){
			int nPartialOperator = myProblem.operators[j]->partialOperator.size();
			FastEnvironment *env = new FastEnvironment(myProblem.operators[j]->originalOperator->parameters->size());
			for (int k= 0; k < nPartialOperator; ++k){
				int nPartialActions = myProblem.operators[j]->partialOperator[k]->child.size();
				int l;
				for (l = 0; l < nPartialActions; ++l){
					if (solver.isTrueAction(myProblem.operators[j]->partialOperator[k]->child[l]->id, i)){
//						sout << ";;Partial action: "; myProblem.operators[j]->partialOperator[k]->child[l]->write(sout); sout << endl;
						break;
					}
				}
				if (l == nPartialActions){
					if (k != 0){
						CANT_HANDLE("Oops, some actions was incompletely done");
					}
					break;
				}else{
					set <const VAL::symbol *>::const_iterator it, itEnd;
					initializeIterator(it, itEnd, myProblem.operators[j]->partialOperator[k]->argument);
					for (; it != itEnd; ++it){
						(*env)[*it] = (*(myProblem.operators[j]->partialOperator[k]->child[l]->env))[*it];
					}
				}
				if (k == nPartialOperator - 1){
					var_symbol_list::iterator paramIt, paramItEnd;
					initializeIterator(paramIt, paramItEnd, (*(myProblem.operators[j]->originalOperator->parameters)));
					for (; paramIt != paramItEnd; ++paramIt){
						if (((*env)[*paramIt]) == NULL){
							//It means some parameters of action is not bind, so I assume, I can bind it to any object!!!
							(*env)[*paramIt] = (*(myProblem.actions[j][0]->valAction->getEnv()))[*paramIt];
						}
					}
					instantiatedOp op (myProblem.operators[j]->originalOperator, env);
					op.write(sout);
					sout << endl;
				}
			}
		}

	}
}

void SATLiftedTranslator::insertSolutionToSMTFormula (LiftedCVC4Problem *smtProblem){
	smtProblem->inActivePermanentChange();
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nOperator; ++j){
			int nPartialOperator = myProblem.operators[j]->partialOperator.size();
			for (int k= 0; k < nPartialOperator; ++k){
				int nPartialActions = myProblem.operators[j]->partialOperator[k]->child.size();
				int l;
				for (l = 0; l < nPartialActions; ++l){
					smtProblem->startNewClause();
					if (solver.isTrueAction(myProblem.operators[j]->partialOperator[k]->child[l]->id, i)){
						smtProblem->addPartialActionToClause(myProblem.operators[j]->partialOperator[k]->child[l], i, true);
					}else{
						smtProblem->addPartialActionToClause(myProblem.operators[j]->partialOperator[k]->child[l], i, false);
					}
					smtProblem->endClause();

				}
			}
		}
	}
}


} /* namespace mdbr */
