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
		addActionMutex(translatedLength - 1);
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
		map <string, MyObject>::iterator objIt, objItEnd;
		objIt = it->objects.begin();
		objItEnd = it->objects.end();
		for (; objIt != objItEnd; ++objIt){
			env[symbol(objIt->first)] = objIt->second.originalObject;
		}

		addAssignmentList(it->partialOperator->assignmentEffect, &env, significantTimePoint + 1, &(*it));
		addGoalList(it->partialOperator->comparisonPrecondition, &env, significantTimePoint, &(*it));
	}
}


//Insert Explanatory Axioms which is needed for SMT problem
void LiftedTranslator::addExplanatoryAxioms (int significantTimePoint){

	list <MyAction *>::iterator actionIt, actionItEnd;

	int nProposition = instantiatedOp::howManyNonStaticLiterals();


	for (int i = 0; i < nProposition; i++){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addConditionToCluase(i, significantTimePoint, false);
		if (isVisited(myProblem.propositions[i].firstVisitedLayer, significantTimePoint)){
			liftedSMTProblem->addConditionToCluase(i, significantTimePoint - 1, true);
			actionIt = myProblem.propositions[i].adderActions.begin();
			actionItEnd = myProblem.propositions[i].adderActions.end();
			for (; actionIt != actionItEnd; ++actionIt){
				if (isVisited((*actionIt)->firstVisitedLayer, significantTimePoint - 1)){
					liftedSMTProblem->addPartialActionToClause((*actionIt)->valAction->getID(), significantTimePoint - 1, true);
				}
			}
		}
		liftedSMTProblem->endClause();
	}

	for (int i = 0; i < nProposition; i++){
		if (!isVisited(myProblem.propositions[i].firstVisitedLayer, significantTimePoint - 1)){
			continue;
		}
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addConditionToCluase(i, significantTimePoint, true);
		liftedSMTProblem->addConditionToCluase(i, significantTimePoint - 1, false);
		actionIt = myProblem.propositions[i].deleterActions.begin();
		actionItEnd = myProblem.propositions[i].deleterActions.end();
		for (; actionIt != actionItEnd; ++actionIt){
			if (isVisited((*actionIt)->firstVisitedLayer, significantTimePoint - 1)){
				liftedSMTProblem->addPartialActionToClause((*actionIt)->valAction->getID(), significantTimePoint - 1, true);
			}
		}
		liftedSMTProblem->endClause();
	}

	int nVariable = instantiatedOp::howManyNonStaticPNEs();
	for (int i = 0; i < nVariable; i++){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->AddEqualityCondition(i, significantTimePoint, i, significantTimePoint - 1, true);
		actionIt = myProblem.variables[i].modifierActions.begin();
		actionItEnd = myProblem.variables[i].modifierActions.end();
		for (; actionIt != actionItEnd; ++actionIt){
			if (isVisited((*actionIt)->firstVisitedLayer, significantTimePoint - 1)){
				liftedSMTProblem->addPartialActionToClause((*actionIt)->valAction->getID(), significantTimePoint - 1, true);
			}
		}
		liftedSMTProblem->endClause();
	}
}

//Inset action mutex to the SMT problem
void LiftedTranslator::addActionMutex (int significantTimePoint){
	int nAction = myProblem.actions.size();
	for (int i = 0; i < nAction; ++i){
		if (!isVisited(myProblem.actions[i].firstVisitedLayer, significantTimePoint)){
			continue;
		}
		set <MyAction *>::const_iterator iter, iterEnd;
		iter = myProblem.actions[i].staticMutex.begin();
		iterEnd = myProblem.actions[i].staticMutex.end();
		for (; iter != iterEnd; ++iter){
			if (myProblem.actions[i].valAction->getID() >= (*iter)->valAction->getID()){
				// Because we want to ensure just one clause for each mutex is inserted so just if the id of first action is less than the second one we inserted the corresponding mutex clause
				continue;
			}
			if (!isVisited((*iter)->firstVisitedLayer, significantTimePoint)){
				continue;
			}
			liftedSMTProblem->startNewClause();
			liftedSMTProblem->addPartialActionToClause(myProblem.actions[i].valAction->getID(), significantTimePoint, false);
			liftedSMTProblem->addPartialActionToClause((*iter)->valAction->getID(), significantTimePoint, false);
			liftedSMTProblem->endClause();
		}
	}
}

void LiftedTranslator::addAtomMutex(int significantTimePoint){
	// Prepare allFoundedAtoms list
	list <MyAtom *> allFoundedAtoms;

	int nPropositions = myProblem.propositions.size();
	for (int i = 0; i < nPropositions; i++){
		if (isVisited(myProblem.propositions[i].firstVisitedLayer, significantTimePoint)){
			allFoundedAtoms.push_back(&(myProblem.propositions[i]));
		}
	}

	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; i++){
		map <double, MyValue>::iterator it, itEnd;
		it = myProblem.variables[i].domain.begin();
		itEnd = myProblem.variables[i].domain.end();
		for (; it != itEnd; ++it){
			if (isVisited(it->second.firstVisitedLayer, significantTimePoint)){
				allFoundedAtoms.push_back(&(it->second));
			}
		}
	}

	list <MyAtom*>::iterator it, itEnd, it2;
	it = allFoundedAtoms.begin();
	itEnd = allFoundedAtoms.end();
	for (; it != itEnd; ++it){
		it2 = allFoundedAtoms.begin();
		for (; it2 != it; ++it2){
			if ((*it)->isMutex(significantTimePoint, *it2)){
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->AddConditionToCluase(*it, significantTimePoint, false);
				liftedSMTProblem->AddConditionToCluase(*it2, significantTimePoint, false);
				liftedSMTProblem->endClause();
			}
		}
	}
}

//Insert the sketchy plan to the SMT problem
void LiftedTranslator::addSkechyPlan(SketchyPlan *sketchyPlan){
	vector < vector < shared_ptr <goal> > > milestones;
	sketchyPlan->convertStateValuesToMilestones(milestones);
	int nStateVariables= milestones.size();
	FastEnvironment env(0);
	for (int i = 1; i < nStateVariables; i++){
		int length = milestones[i].size();
		for (int j = 1; j < length; j++){
			addGoal(milestones[i][j].get(), &env, j);
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


int LiftedTranslator::solve(SketchyPlan *sketchyPlan){

	//create assertions for intermediate and final goals

	liftedSMTProblem->inActivePermanentChange();
	liftedSMTProblem->clearAssertionList();
	if (sketchyPlan != NULL){
		addSkechyPlan(sketchyPlan);
	}
	liftedSMTProblem->insertAssertion(goals);
	Expr translatedGoals = liftedSMTProblem->getAssertions();

	//try to solve the problem
	return liftedSMTProblem->solve(translatedGoals);
}

bool LiftedTranslator::solve(){
	double ret = solve(NULL);
	if (ret == numeric_limits <int>::max()) {
		return true;
	}
	return false;
}

