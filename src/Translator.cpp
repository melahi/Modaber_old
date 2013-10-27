#include "CVC4Problem.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "Utilities.h"
#include "Translator.h"
#include "MyTimer.h"
#include "MyAtom.h"
#include <ptree.h>
#include <limits>

using namespace VAL;
using namespace Inst;


using namespace mdbr;


void Translator::prepareGoals() {
	smtProblem->inActivePermanentChange();
	smtProblem->clearAssertionList();
	addGoals(translatedLength - 1);
	goals = smtProblem->getAssertions();
}

void Translator::prepare (int length){
	if (length == 1 && translatedLength == 1) {
		prepareGoals();
		return;
	}

	if (translatedLength >= length){
		CANT_HANDLE("prepare function is called with the smaller number of length than it is translated");
		return;
	}

	smtProblem->guaranteeSize(length);
	smtProblem->activePermanentChange();
	for (; translatedLength < length; translatedLength++){
		addActions(translatedLength - 1);
		addActionMutex(translatedLength - 1);
		addExplanatoryAxioms(translatedLength);
		addAtomMutex(translatedLength);
	}
	smtProblem->inActivePermanentChange();

	//Find goals expression
	prepareGoals();
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
		env = (*iter)->getEnv();
		if (isVisited(myProblem.actions[(*iter)->getID()].firstVisitedLayer, significantTimePoint)){
			addEffectList((*iter)->forOp()->effects, env, significantTimePoint + 1, (*iter)->getID());
			addGoal((*iter)->forOp()->precondition, env, significantTimePoint, (*iter)->getID());
		}
	}
}


//Insert Explanatory Axioms which is needed for SMT problem
void Translator::addExplanatoryAxioms (int significantTimePoint){

	list <MyAction *>::iterator actionIt, actionItEnd;

	int nProposition = instantiatedOp::howManyNonStaticLiterals();


	for (int i = 0; i < nProposition; i++){
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(i, significantTimePoint, false);
		if (isVisited(myProblem.propositions[i].firstVisitedLayer, significantTimePoint)){
			smtProblem->addConditionToCluase(i, significantTimePoint - 1, true);
			actionIt = myProblem.propositions[i].adderActions.begin();
			actionItEnd = myProblem.propositions[i].adderActions.end();
			for (; actionIt != actionItEnd; ++actionIt){
				if (isVisited((*actionIt)->firstVisitedLayer, significantTimePoint - 1)){
					smtProblem->addActionToClause((*actionIt)->valAction->getID(), significantTimePoint - 1, true);
				}
			}
		}
		smtProblem->endClause();
	}

	for (int i = 0; i < nProposition; i++){
		if (!isVisited(myProblem.propositions[i].firstVisitedLayer, significantTimePoint - 1)){
			continue;
		}
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(i, significantTimePoint, true);
		smtProblem->addConditionToCluase(i, significantTimePoint - 1, false);
		actionIt = myProblem.propositions[i].deleterActions.begin();
		actionItEnd = myProblem.propositions[i].deleterActions.end();
		for (; actionIt != actionItEnd; ++actionIt){
			if (isVisited((*actionIt)->firstVisitedLayer, significantTimePoint - 1)){
				smtProblem->addActionToClause((*actionIt)->valAction->getID(), significantTimePoint - 1, true);
			}
		}
		smtProblem->endClause();
	}

	int nVariable = instantiatedOp::howManyNonStaticPNEs();
	for (int i = 0; i < nVariable; i++){
		smtProblem->startNewClause();
		smtProblem->AddEqualityCondition(i, significantTimePoint, i, significantTimePoint - 1);
		actionIt = myProblem.variables[i].modifierActions.begin();
		actionItEnd = myProblem.variables[i].modifierActions.end();
		for (; actionIt != actionItEnd; ++actionIt){
			if (isVisited((*actionIt)->firstVisitedLayer, significantTimePoint - 1)){
				smtProblem->addActionToClause((*actionIt)->valAction->getID(), significantTimePoint - 1, true);
			}
		}
		smtProblem->endClause();
	}
}

//Inset action mutex to the SMT problem
void Translator::addActionMutex (int significantTimePoint){
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
			smtProblem->startNewClause();
			smtProblem->addActionToClause(myProblem.actions[i].valAction->getID(), significantTimePoint, false);
			smtProblem->addActionToClause((*iter)->valAction->getID(), significantTimePoint, false);
			smtProblem->endClause();
		}
	}
}

void Translator::addAtomMutex(int significantTimePoint){
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
				smtProblem->startNewClause();
				smtProblem->AddConditionToCluase(*it, significantTimePoint, false);
				smtProblem->AddConditionToCluase(*it2, significantTimePoint, false);
				smtProblem->endClause();
			}
		}
	}
}

//Insert the sketchy plan to the SMT problem
void Translator::addSkechyPlan(SketchyPlan *sketchyPlan){
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


void Translator::addSimpleEffectList (polarity plrty, const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env, int significantTimePoint, int actionID /* = -1 */){
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

void Translator::addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, int actionID /* = -1 */){
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

void Translator::addEffectList (const effect_lists *effects, FastEnvironment *env, int significantTimePoint, int actionId /* = -1 */){
	addSimpleEffectList(E_POS, effects->add_effects, env, significantTimePoint, actionId);
	addSimpleEffectList(E_NEG, effects->del_effects, env, significantTimePoint, actionId);
	addAssignmentList(effects->assign_effects, env, significantTimePoint, actionId);
	if ((!effects->forall_effects.empty()) || (!effects->cond_effects.empty()) || (!effects->cond_assign_effects.empty()) || (!effects->timed_effects.empty())){
		CANT_HANDLE("Some kinds of Effects");
	}
}

void Translator::addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, int actionId /* = -1 */){
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
	CANT_HANDLE("can't translate some GOAL");
}

//bool firstTime = true;

int Translator::solve(SketchyPlan *sketchyPlan){

	//create assertions for intermediate and final goals

	smtProblem->inActivePermanentChange();
	smtProblem->clearAssertionList();
	if (sketchyPlan != NULL){
		addSkechyPlan(sketchyPlan);
	}
	smtProblem->insertAssertion(goals);
	Expr translatedGoals = smtProblem->getAssertions();

	//try to solve the problem
	return smtProblem->solve(translatedGoals);
}

bool Translator::solve(){
	double ret = solve(NULL);
	if (ret == numeric_limits <int>::max()) {
		return true;
	}
	return false;
}

