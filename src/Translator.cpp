#include "CVC4Problem.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "MyAnalyzer.h"
#include "Utilities.h"
#include "Translator.h"
#include "MyTimer.h"
#include <ptree.h>

using namespace VAL;
using namespace Inst;



void Translator::prepare (int length){
	if (translatedLength >= length){
		CANT_HANDLE("prepare function is called with the smaller number of length than it is translated");
		return;
	}
	smtProblem->guaranteeSize(length);
	for (; translatedLength < length; translatedLength++){
		smtProblem->clearAssertionList();
		addActions(translatedLength - 1);
		addActionMutex(translatedLength - 1);
		addExplanatoryAxioms(translatedLength);
		smtProblem->assertFormula();
	}

	//Find goals expression
	smtProblem->clearAssertionList();
	addGoals(translatedLength - 1);
	goals = smtProblem->getAssertions();
}


//Translate initial state of planning problem to SMT problem
void Translator::addInitialState(){
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
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(i, 0, initialState[i]);
		smtProblem->endClause();
	}

	addAssignmentList(current_analysis->the_problem->initial_state->assign_effects, &env, 0);
}

//Add goals to the smt problem
void Translator::addGoals (int significantTimePoint){
	FastEnvironment env(0);
	addGoal(current_analysis->the_problem->the_goal, &env, significantTimePoint);
}


//Insert actions' conditions for the specified time point in smt problem
void Translator::addActions (int significantTimePoint){
	OpStore::const_iterator iter, iterEnd;
	iter = instantiatedOp::opsBegin();
	iterEnd = instantiatedOp::opsEnd();
	FastEnvironment *env;
	for (; iter != iterEnd; ++iter){
		if (numericRPG->firstVisitedAcotion[(*iter)->getID()] <= significantTimePoint){
			env = (*iter)->getEnv();
			addEffectList((*iter)->forOp()->effects, env, significantTimePoint + 1, (*iter)->getID());
			addGoal((*iter)->forOp()->precondition, env, significantTimePoint, (*iter)->getID());
		}
	}
}


//Insert Explanatory Axioms which is needed for SMT problem
void Translator::addExplanatoryAxioms (int significantTimePoint){
	int nProposition = instantiatedOp::howManyNonStaticLiterals();
	for (int i = 0; i < nProposition; i++){
		int nAdder = myAnalyzer->adderActions[i].size();
		if (nAdder > 0){
			smtProblem->startNewClause();
			smtProblem->addConditionToCluase(i, significantTimePoint, false);
			if (numericRPG->firstVisitedProposition[i] <= significantTimePoint){
				smtProblem->addConditionToCluase(i, significantTimePoint - 1, true);
				for (int j = 0; j < nAdder; j++){
					if (numericRPG->firstVisitedAcotion[myAnalyzer->adderActions[i][j]] < significantTimePoint){
						smtProblem->addActionToClause(myAnalyzer->adderActions[i][j], significantTimePoint - 1, true);
					}
				}
			}
			smtProblem->endClause();
		}
	}

	for (int i = 0; i < nProposition; i++){
		if (numericRPG->firstVisitedProposition[i] >= significantTimePoint){
			continue;
		}
		int nDeleter = myAnalyzer->deleterActions[i].size();
		if (nDeleter > 0){
			smtProblem->startNewClause();
			smtProblem->addConditionToCluase(i, significantTimePoint, true);
			smtProblem->addConditionToCluase(i, significantTimePoint - 1, false);
			for (int j = 0; j < nDeleter; j++){
				if (numericRPG->firstVisitedAcotion[myAnalyzer->deleterActions[i][j]] < significantTimePoint){
					smtProblem->addActionToClause(myAnalyzer->deleterActions[i][j], significantTimePoint - 1, true);
				}
			}
			smtProblem->endClause();
		}
	}

	int nVariable = instantiatedOp::howManyNonStaticPNEs();
	for (int i = 0; i < nVariable; i++){
		int nModifier = myAnalyzer->variableModifierActions[i].size();
		if (nModifier > 0){
			smtProblem->startNewClause();
			smtProblem->AddEqualityCondition(i, significantTimePoint, i, significantTimePoint - 1);
			for (int j = 0; j < nModifier; j++){
				if (numericRPG->firstVisitedAcotion[myAnalyzer->variableModifierActions[i][j]] < significantTimePoint){
					smtProblem->addActionToClause(myAnalyzer->variableModifierActions[i][j], significantTimePoint - 1, true);
				}
			}
			smtProblem->endClause();
		}
	}
}

//Inset action mutex to the SMT problem
void Translator::addActionMutex (int significantTimePoint){
	int nAction = myAnalyzer->mutexActions.size();
	for (int i = 0; i < nAction; ++i){
		if (numericRPG->firstVisitedAcotion[i] > significantTimePoint){
			continue;
		}
		set <int>::const_iterator iter, iterEnd;
		iter = myAnalyzer->mutexActions[i].begin();
		iterEnd = myAnalyzer->mutexActions[i].end();
		for (; iter != iterEnd; iter++){
			if (i >= *iter){
				// Because we want to ensure just one clause for each mutex is inserted so just if the id of first action is less than the second one we inserted the corresponding mutex clause
				continue;
			}
			if (numericRPG->firstVisitedAcotion[*iter] > significantTimePoint){
				continue;
			}
			smtProblem->startNewClause();
			smtProblem->addActionToClause(i, significantTimePoint, false);
			smtProblem->addActionToClause(*iter, significantTimePoint, false);
			smtProblem->endClause();
		}
	}
}

//Insert the sketchy plan to the SMT problem
void Translator::addSkechyPlan(SketchyPlan *sketchyPlan){
	int length = sketchyPlan->milestones.size();
	FastEnvironment env(0);
	for (int i = 0; i < length; i++){
		int mySize = sketchyPlan->milestones[i].size();
		for (int j = 0; j < mySize; j++){
			addGoal(sketchyPlan->milestones[i][j].get(), &env, i);
		}
	}
}


void Translator::addSimpleEffectList (polarity plrty, const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env, int significantTimePoint, int actionID){
	pc_list<simple_effect*>::const_iterator it = simpleEffectList.begin();
	pc_list<simple_effect*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		smtProblem->startNewClause();
		if (actionID != -1){
			smtProblem->addActionToClause(actionID, significantTimePoint - 1, false);
		}
		smtProblem->addLiteral(plrty, (*it)->prop, env, significantTimePoint);
		smtProblem->endClause();
	}
}

void Translator::addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, int actionID){
	pc_list<assignment*>::const_iterator it = assignmentEffects.begin();
	pc_list<assignment*>::const_iterator itEnd = assignmentEffects.end();
	for (; it != itEnd; ++it){
		smtProblem->startNewClause();
		if (actionID != -1){
			smtProblem->addActionToClause(actionID, significantTimePoint - 1, false);
		}
		smtProblem->AddConditionToCluase(*it, env, significantTimePoint);
		smtProblem->endClause();
	}
}

void Translator::addEffectList (const effect_lists *effects, FastEnvironment *env, int significantTimePoint, int actionId){
	addSimpleEffectList(E_POS, effects->add_effects, env, significantTimePoint, actionId);
	addSimpleEffectList(E_NEG, effects->del_effects, env, significantTimePoint, actionId);
	addAssignmentList(effects->assign_effects, env, significantTimePoint, actionId);
	if ((!effects->forall_effects.empty()) || (!effects->cond_effects.empty()) || (!effects->cond_assign_effects.empty()) || (!effects->timed_effects.empty())){
		CANT_HANDLE("Some kinds of Effects");
	}
}

void Translator::addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, int actionId){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		smtProblem->startNewClause();
		if (actionId != -1){
			smtProblem->addActionToClause(actionId, significantTimePoint, false);
		}
		smtProblem->addLiteral(simple->getPolarity(), simple->getProp(),env, significantTimePoint);
		smtProblem->endClause();
		return;
	}
	const comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		smtProblem->startNewClause();
		if (actionId != -1){
			smtProblem->addActionToClause(actionId, significantTimePoint, false);
		}
		smtProblem->AddConditionToCluase(comp, env, significantTimePoint);
		smtProblem->endClause();
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			addGoal(*it, env, significantTimePoint, actionId);
		}
		return;
	}
	CANT_HANDLE("translating some GOAL");
}

bool firstTime = true;

double Translator::solve(SketchyPlan *sketchyPlan){

	//create assertions for intermediate and final goals
	smtProblem->clearAssertionList();
	addSkechyPlan(sketchyPlan);
	smtProblem->insertAssertion(goals);
	Expr translatedGoals = smtProblem->getAssertions();

	//try to solve the problem
	return smtProblem->solve(translatedGoals);
}
