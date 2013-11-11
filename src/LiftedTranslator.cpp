#include "LiftedCVC4Problem.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "Utilities.h"
#include "LiftedTranslator.h"
#include "MyTimer.h"
#include "MyAtom.h"
#include <ptree.h>
#include <limits>

using namespace VAL;
using namespace Inst;


using namespace mdbr;


void LiftedTranslator::prepareGoals() {
	liftedSMTProblem->inActivePermanentChange();
	liftedSMTProblem->clearAssertionList();
	addGoals(translatedLength - 1);
	goals = liftedSMTProblem->getAssertions();
}

void LiftedTranslator::prepare (int length){
	if (length == 1 && translatedLength == 1) {
		prepareGoals();
		return;
	}

	if (translatedLength >= length){
		CANT_HANDLE("prepare function is called with the smaller number of length than it is translated");
		return;
	}

	liftedSMTProblem->guaranteeSize(length);
	liftedSMTProblem->activePermanentChange();
	for (; translatedLength < length; translatedLength++){
		addPartialActions(translatedLength - 1);
		addExplanatoryAxioms(translatedLength);
		addAtomMutex(translatedLength);
	}
	liftedSMTProblem->inActivePermanentChange();

	//Find goals expression
	prepareGoals();
}


//Translate initial state of planning problem to SMT problem
void LiftedTranslator::addInitialState(){
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
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addConditionToCluase(i, 0, 0, initialState[i]);
		liftedSMTProblem->endClause();
	}

	addAssignmentList(current_analysis->the_problem->initial_state->assign_effects, &env, 0);
}

//Add goals to the smt problem
void LiftedTranslator::addGoals (int significantTimePoint){
	FastEnvironment env(0);
	addGoal(current_analysis->the_problem->the_goal, &env, significantTimePoint);
}


//Insert actions' conditions for the specified time point in smt problem
void LiftedTranslator::addPartialActions (int significantTimePoint){
	list <MyPartialAction>::iterator it, itEnd;
	it = myProblem.partialAction.begin();
	itEnd = myProblem.partialAction.end();
	for (; it != itEnd; ++it){
		if (it->isValid == false){
			liftedSMTProblem->startNewClause();
			liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, false);
			liftedSMTProblem->endClause();
			continue;
		}

		addSimpleEffectList(E_POS, it->addEffect, significantTimePoint + 1, &(*it));
		addSimpleEffectList(E_NEG, it->deleteEffect, significantTimePoint + 1, &(*it));
		addGoalList(it->precondition, significantTimePoint, &(*it));

		if (it->partialOperator->comparisonPrecondition.size() == 0 && it->partialOperator->assignmentEffect.size() == 0){
			continue;
		}
		//Creating FastEnvironment;
		FastEnvironment env(it->objects.size());
		map <string, MyObject *>::iterator objIt, objItEnd;
		objIt = it->objects.begin();
		objItEnd = it->objects.end();
		for (; objIt != objItEnd; ++objIt){
			env[&(symbol(objIt->first))] = objIt->second->originalObject;
		}

		addAssignmentList(it->partialOperator->assignmentEffect, &env, significantTimePoint + 1, &(*it));
		addGoalList(it->partialOperator->comparisonPrecondition, &env, significantTimePoint, &(*it));
	}
}


//Insert Explanatory Axioms which is needed for SMT problem
void LiftedTranslator::addExplanatoryAxioms (int significantTimePoint){

	list <MyPartialAction* >::iterator pAIt, pAItEnd;

	int nProposition = myProblem.propositions.size();
	int nOperators = myProblem.operators.size();

	for (int i = 0; i < nProposition; i++){

		vector < list<MyPartialAction *> > adder(nOperators);
		pAIt = myProblem.propositions[i].adder.begin();
		pAItEnd = myProblem.propositions[i].adder.end();
		for (; pAIt != pAItEnd; ++pAIt){
			adder[(*pAIt)->op->id].push_back(*pAIt);
		}

		vector < list<MyPartialAction *> > deleter(nOperators);
		pAIt = myProblem.propositions[i].deleter.begin();
		pAItEnd = myProblem.propositions[i].deleter.end();
		for (; pAIt != pAItEnd; ++pAIt){
			deleter[(*pAIt)->op->id].push_back(*pAIt);
		}


		for (int j = 0; j < nOperators; ++j){
			if (adder[j].size() || deleter[j].size()){

				//Deleter actions
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addConditionToCluase(i, j, significantTimePoint, false);
				liftedSMTProblem->addConditionToCluase(i, j + 1, significantTimePoint, true);
				pAIt = deleter[j].begin();
				pAItEnd = deleter[j].end();
				for (; pAIt != pAItEnd; ++pAIt){
					liftedSMTProblem->addPartialActionToClause(*pAIt, significantTimePoint, true);
				}
				liftedSMTProblem->endClause();

				//Adder Actions
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addConditionToCluase(i, j, significantTimePoint, true);
				liftedSMTProblem->addConditionToCluase(i, j + 1, significantTimePoint, false);
				pAIt = adder[j].begin();
				pAItEnd = adder[j].end();
				for (; pAIt != pAItEnd; ++pAIt){
					liftedSMTProblem->addPartialActionToClause(*pAIt, significantTimePoint, true);
				}
				liftedSMTProblem->endClause();
			}
		}
	}



	int nVariable = myProblem.variables.size();
	for (int i = 0; i < nVariable; i++){
		vector < list<MyPartialAction *> > modifier(nOperators);
		pAIt = myProblem.variables[i].modifier.begin();
		pAItEnd = myProblem.variables[i].modifier.end();
		for (; pAIt != pAItEnd; ++pAIt){
			modifier[(*pAIt)->op->id].push_back(*pAIt);
		}
		for (int j = 0; j < nOperators; ++j){
			if (modifier[j].size()){
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->AddEqualityCondition(i, j, significantTimePoint, i, j+1, significantTimePoint, true);
				pAIt = modifier[j].begin();
				pAItEnd = modifier[j].end();
				for (; pAIt != pAItEnd; ++pAIt){
					liftedSMTProblem->addPartialActionToClause(*pAIt, significantTimePoint, true);
				}
				liftedSMTProblem->endClause();
			}
		}
	}
}

void LiftedTranslator::addAtomMutex(int significantTimePoint){
	int nProposition = myProblem.propositions.size();
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < nProposition; ++i){
		for (int j = 0; j < i; ++j){
			if (myProblem.propositions[i].isMutex ((1 << 20), &(myProblem.propositions[j]))){
				int lastId1, lastId2;
				lastId1 = lastId2 = -1;
				for (int k = 0; k < nOperator; ++k){
					if (myProblem.propositions[i].ids[k] == lastId1 && myProblem.propositions[j].ids[k]){
						continue;
					}
					liftedSMTProblem->startNewClause();
					liftedSMTProblem->addConditionToCluase(i, k, significantTimePoint, false);
					liftedSMTProblem->addConditionToCluase(j, k, significantTimePoint, false);
					liftedSMTProblem->endClause();
					lastId1 = myProblem.propositions[i].ids[k];
					lastId2 = myProblem.propositions[j].ids[k];
				}
			}
		}
	}
}

void LiftedTranslator::addSimpleEffectList (polarity plrty, const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction){
	list <MyProposition*>::const_iterator it = simpleEffectList.begin();
	list <MyProposition*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint - 1, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->op->id, significantTimePoint, (plrty == E_POS));
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction /* = NULL */){
	pc_list<assignment*>::const_iterator it = assignmentEffects.begin();
	pc_list<assignment*>::const_iterator itEnd = assignmentEffects.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		if (partialAction){
			liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint - 1, false);
			liftedSMTProblem->AddConditionToCluase(*it, env, partialAction->op->id, significantTimePoint);
		}else{
			liftedSMTProblem->AddConditionToCluase(*it, env, 0, significantTimePoint);
		}
		liftedSMTProblem->endClause();
	}
}


void LiftedTranslator::addAssignmentList (const list <const assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction){
	list <const assignment*>::const_iterator it = assignmentEffects.begin();
	list <const assignment*>::const_iterator itEnd = assignmentEffects.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint - 1, false);
		liftedSMTProblem->AddConditionToCluase(*it, env, partialAction->op->id, significantTimePoint);
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction /* = NULL */){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		liftedSMTProblem->startNewClause();
		if (partialAction){
			liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
			liftedSMTProblem->addLiteral(simple->getPolarity(), simple->getProp(),env, partialAction->op->id, significantTimePoint);
		}else{
			liftedSMTProblem->addLiteral(simple->getPolarity(), simple->getProp(),env, 0, significantTimePoint);
		}
		liftedSMTProblem->endClause();
		return;
	}
	const comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		liftedSMTProblem->startNewClause();
		if (partialAction){
			liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
			liftedSMTProblem->AddConditionToCluase(comp, env, partialAction->op->id, significantTimePoint);
		}else{
			liftedSMTProblem->AddConditionToCluase(comp, env, 0, significantTimePoint);
		}
		liftedSMTProblem->endClause();
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
	CANT_HANDLE("can't translate some GOAL");
}

void LiftedTranslator::addGoalList (const list <const comparison *> &gl, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction){
	list <const comparison *>::const_iterator it, itEnd;
	it = gl.begin();
	itEnd = gl.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, env, partialAction->op->id, significantTimePoint);
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::addGoalList (const list <MyProposition *> &preconditionList, int significantTimePoint, MyPartialAction *partialAction){
	list <MyProposition*>::const_iterator it = preconditionList.begin();
	list <MyProposition*>::const_iterator itEnd = preconditionList.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->op->id, significantTimePoint, true);
		liftedSMTProblem->endClause();
	}
}


bool LiftedTranslator::solve(){


	//try to solve the problem
	return liftedSMTProblem->solve(goals);
}
