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
		addCompletingAction(translatedLength - 1);
		addExplanatoryAxioms(translatedLength - 1);
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

		if (it->unificationId.size() != 0){
			map <string, int>::iterator it1, it1End;
			it1 = it->unificationId.begin();
			it1End = it->unificationId.end();
			liftedSMTProblem->startNewClause();
			for (; it1 != it1End; ++it1){
				liftedSMTProblem->addUnificationToClause(it1->second + it->op->offset[it->partialOperator->placement[it1->first]], significantTimePoint, false);
			}
			liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, true);
			liftedSMTProblem->endClause();

			it1 = it->unificationId.begin();
			for (; it1 != it1End; ++it1){
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, false);
				liftedSMTProblem->addUnificationToClause(it1->second + it->op->offset[it->partialOperator->placement[it1->first]], significantTimePoint, true);
				liftedSMTProblem->endClause();
			}
		}else{
			if (it->op->argument.size() == 0){
				CANT_HANDLE("AN OPERATOR WITH NO ARGUMENT, I DON'T HAVE ANY PLAN FOR IT!!!");
				exit(1);
			}
			int unificationId = it->op->offset[0];
			int endingUnificationID = it->op->argument[0]->objects.size() + unificationId;
			for (; unificationId < endingUnificationID; ++unificationId){
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addUnificationToClause(unificationId, significantTimePoint, false);
				liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, true);
				liftedSMTProblem->endClause();
			}


			liftedSMTProblem->startNewClause();
			unificationId = it->op->offset[0];
			liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, false);
			for (; unificationId < endingUnificationID; ++unificationId){
				liftedSMTProblem->addUnificationToClause(unificationId, significantTimePoint, true);
			}
			liftedSMTProblem->endClause();
		}



		if (it->isValid == false){
			liftedSMTProblem->startNewClause();
			liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, false);
			liftedSMTProblem->endClause();
			continue;
		}

		addSimpleEffectList(E_POS, it->addEffect, significantTimePoint, &(*it));
		addSimpleEffectList(E_NEG, it->deleteEffect, significantTimePoint, &(*it));
		addGoalList(it->precondition, significantTimePoint, &(*it));

		if (it->partialOperator->comparisonPrecondition.size() == 0 && it->partialOperator->assignmentEffect.size() == 0){
			continue;
		}
		//Creating FastEnvironment;
		FastEnvironment env(it->op->originalOperator->parameters->size());

		var_symbol_list::iterator varIt, varItEnd;
		varIt = it->op->originalOperator->parameters->begin();
		varItEnd = it->op->originalOperator->parameters->end();
		for (; varIt != varItEnd; ++varIt){
			if (it->objects.find((*varIt)->getName()) != it->objects.end()){
				env[*varIt] = it->objects[(*varIt)->getName()]->originalObject;
			}
		}

		addAssignmentList(it->partialOperator->assignmentEffect, &env, significantTimePoint, &(*it));
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
			if (adder[j].size() || deleter[j].size() || (j == 0 && myProblem.propositions[i].adder.size() + myProblem.propositions[i].deleter.size() == 0)){

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
			if (modifier[j].size() || (j == 0 && myProblem.variables[i].modifier.size() == 0)){
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

void LiftedTranslator::addCompletingAction (int significantTimePoint){
	int nOperators = myProblem.operators.size();
	for (int i = 0; i < nOperators; ++i){
		int nArguments = myProblem.operators[i]->argument.size();
		if (nArguments <= 1){
			continue;
		}
		for (int j = 0; j < nArguments; ++j){
			int starting1, starting2, ending1, ending2;
			starting1 = myProblem.operators[i]->offset[j];
			ending1 = starting1 + myProblem.operators[i]->argument[j]->objects.size();
			starting2 = myProblem.operators[i]->offset[(j + 1) % nArguments];
			ending2 = starting2 + myProblem.operators[i]->argument[(j + 1) % nArguments]->objects.size();

			//If a unification for an argument is true then at least one unification for the next argument is also should be true
			for (int k = starting1; k < ending1; ++k){
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addUnificationToClause(k, significantTimePoint, false);
				for (int l = starting2; l < ending2; ++l){
					liftedSMTProblem->addUnificationToClause(l, significantTimePoint, true);
				}
				liftedSMTProblem->endClause();
			}

			//At most on unification for each argument should be true
			for (int k = starting1; k < ending1; ++k){
				for (int l = starting1; l < k; ++ l){
					liftedSMTProblem->startNewClause();
					liftedSMTProblem->addUnificationToClause(k, significantTimePoint, false);
					liftedSMTProblem->addUnificationToClause(l, significantTimePoint, false);
					liftedSMTProblem->endClause();
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
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->op->id + 1, significantTimePoint, (plrty == E_POS));
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
			liftedSMTProblem->AddConditionToCluase(*it, env, partialAction->op->id + 1, significantTimePoint);
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
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, env, partialAction->op->id + 1, significantTimePoint);
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


void LiftedTranslator::extractSolution (ostream &sout){
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nOperator; ++j){
			int nArgument = myProblem.operators[j]->argument.size();
			for (int k = 0; k < nArgument; ++k){
				int offset = myProblem.operators[j]->offset[k];
				int nUnification = myProblem.operators[j]->argument[k]->objects.size();
				int objectId;
				for (objectId = 0; objectId < nUnification; ++objectId){
					if (liftedSMTProblem->isUnificationUsed(offset + objectId, i)){
						break;
					}
				}
				if (k == 0){
					if (objectId == nUnification){
						break;
					}
					sout << "(" << myProblem.operators[j]->originalOperator->name->getName();
				}
				sout << " " << myProblem.operators[j]->argument[k]->objects[objectId]->originalObject->getName();
				if (k == nArgument - 1){
					sout << ")" << endl;
				}
			}
		}
	}
}
