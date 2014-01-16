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




void LiftedTranslator::prepareGoals(double bound) {
	liftedSMTProblem->inActivePermanentChange();
	liftedSMTProblem->clearAssertionList();
	addGoals(translatedLength - 1);
	if (bound != infinite && bound != -infinite){
		addMetric(bound, translatedLength - 1);
	}
	goals = liftedSMTProblem->getAssertions();
}

void LiftedTranslator::prepare (int length, double bound){
	if (length == 1 && translatedLength == 1) {
		prepareGoals(bound);
		return;
	}

	if (translatedLength > length){
		CANT_HANDLE("prepare function is called with the smaller number of length than it is translated");
		return;
	}

	if (translatedLength == length){
		prepareGoals(bound);
		return;
	}

	liftedSMTProblem->guaranteeSize(length);
	liftedSMTProblem->activePermanentChange();
	for (; translatedLength < length; translatedLength++){
		addPartialActions(translatedLength - 1);
		addCompletingAction(translatedLength - 1);
		addExplanatoryAxioms(translatedLength - 1);
		addAtomMutex(translatedLength - 1);
	}
	liftedSMTProblem->inActivePermanentChange();

	//Find goals expression
	prepareGoals(bound);
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
		if (myProblem.propositions[i].possibleEffective){
			liftedSMTProblem->startNewClause();
			liftedSMTProblem->addConditionToCluase(i, 0, 0, initialState[i]);
			liftedSMTProblem->endClause();
		}
	}

	addAssignmentList(current_analysis->the_problem->initial_state->assign_effects, 0);
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

		MyOperator *theOperator = it->partialOperator->op;
		if (it->partialOperator->argument.size() != 0){
			set <const VAL::symbol*>::iterator it1, it1End;

			FOR_ITERATION(it1, it1End, it->partialOperator->argument){
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addPartialActionToClause(&(*it), significantTimePoint, false);
				liftedSMTProblem->addUnificationToClause(theOperator->unificationId[theOperator->argument[*it1]][(*(it->env))[*it1]->getName()], significantTimePoint, true);
				liftedSMTProblem->endClause();
			}
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
		addAssignmentList(it->partialOperator->assignmentEffect, significantTimePoint, &(*it));
		addGoalList(it->partialOperator->comparisonPrecondition, significantTimePoint, &(*it));
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
			modifier[(*pAIt)->partialOperator->op->id].push_back(*pAIt);
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
		for (int j = 0; j < nArguments; ++j){

			map <string, int>::iterator it1, it2, itEnd1, itEnd2;

			//At most on unification for each argument should be true
			FOR_ITERATION(it1, itEnd1, myProblem.operators[i]->unificationId[j]) {
				it2 = myProblem.operators[i]->unificationId[j].begin();
				for (; it2 != it1; ++it2){
					liftedSMTProblem->startNewClause();
					liftedSMTProblem->addUnificationToClause(it1->second, significantTimePoint, false);
					liftedSMTProblem->addUnificationToClause(it2->second, significantTimePoint, false);
					liftedSMTProblem->endClause();
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
				liftedSMTProblem->startNewClause();
				liftedSMTProblem->addPartialActionToClause(myProblem.operators[i]->partialOperator[j]->child[k], significantTimePoint, false);
				for (int l = 0; l < nNextPartialActions; ++l){
					liftedSMTProblem->addPartialActionToClause(myProblem.operators[i]->partialOperator[nextPartialOperator]->child[l], significantTimePoint, true);
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
			for (int k = 0; k < nOperator; ++k){
				int layerNumber = (significantTimePoint * nOperator) + k;
				if (myProblem.propositions[i].isMutex (layerNumber, &(myProblem.propositions[j]))){
					int lastId1, lastId2;
					lastId1 = lastId2 = -1;
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


void LiftedTranslator::addMetric (double bound, int significantTimePoint){
	comparison_op compOp;
	if (current_analysis->the_problem->metric->opt == E_MINIMIZE){
		compOp = E_LESS;
	}else{
		compOp = E_GREATER;
	}
	FastEnvironment env(0);
	liftedSMTProblem->startNewClause();
	liftedSMTProblem->AddConditionToCluase(current_analysis->the_problem->metric->expr, &env, 0, compOp, bound, significantTimePoint);
	liftedSMTProblem->endClause();
}

void LiftedTranslator::addSimpleEffectList (polarity plrty, const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction){
	list <MyProposition*>::const_iterator it = simpleEffectList.begin();
	list <MyProposition*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->partialOperator->op->id + 1, significantTimePoint, (plrty == E_POS));
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::addAssignmentList (const pc_list <assignment *> &assignmentEffects, int significantTimePoint, MyPartialAction *partialAction /* = NULL */){
	pc_list<assignment*>::const_iterator it = assignmentEffects.begin();
	pc_list<assignment*>::const_iterator itEnd = assignmentEffects.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		if (partialAction){
			liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint - 1, false);
			liftedSMTProblem->AddConditionToCluase(*it, partialAction->env, partialAction->partialOperator->op->id + 1, significantTimePoint);
		}else{
			FastEnvironment env(0);
			liftedSMTProblem->AddConditionToCluase(*it, &env, 0, significantTimePoint);
		}
		liftedSMTProblem->endClause();
	}
}


void LiftedTranslator::addAssignmentList (const list <const assignment *> &assignmentEffects, int significantTimePoint, MyPartialAction *partialAction){
	list <const assignment*>::const_iterator it = assignmentEffects.begin();
	list <const assignment*>::const_iterator itEnd = assignmentEffects.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->env, partialAction->partialOperator->op->id + 1, significantTimePoint);
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction /* = NULL */){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		liftedSMTProblem->startNewClause();
		if (partialAction){
			liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
			liftedSMTProblem->addLiteral(simple->getPolarity(), simple->getProp(),env, partialAction->partialOperator->op->id, significantTimePoint);
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
			liftedSMTProblem->AddConditionToCluase(comp, env, partialAction->partialOperator->op->id, significantTimePoint);
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
	const preference *thePreference = dynamic_cast <const preference *> (gl);
	if (thePreference){
		list <const simple_goal *> softGoals;
		findGoalList(thePreference->getGoal(), softGoals);
		list <const simple_goal *>::iterator it, itEnd;
		itEnd = softGoals.end();
		Expr preferenceExpr = liftedSMTProblem->getPreferenceExpr(thePreference->getName());
		it = softGoals.begin();
		liftedSMTProblem->startNewClause();
		for (; it != itEnd; ++it){
			liftedSMTProblem->addLiteral((((*it)->getPolarity() == E_POS) ? E_NEG : E_POS), (*it)->getProp(), env, 0, significantTimePoint);
		}
		liftedSMTProblem->AddEqualityCondition(preferenceExpr, 0);
		liftedSMTProblem->endClause();
		it = softGoals.begin();
		for (; it != itEnd; ++it){
			liftedSMTProblem->startNewClause();
			liftedSMTProblem->addLiteral((*it)->getPolarity(), (*it)->getProp(), env, 0, significantTimePoint);
			liftedSMTProblem->AddEqualityCondition(preferenceExpr, 1);
			liftedSMTProblem->endClause();
		}
		return;
	}

	CANT_HANDLE("can't translate some GOAL");
}

void LiftedTranslator::addGoalList (const list <const comparison *> &gl, int significantTimePoint, MyPartialAction *partialAction){
	list <const comparison *>::const_iterator it, itEnd;
	it = gl.begin();
	itEnd = gl.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->env, partialAction->partialOperator->op->id, significantTimePoint);
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::addGoalList (const list <MyProposition *> &preconditionList, int significantTimePoint, MyPartialAction *partialAction){
	list <MyProposition*>::const_iterator it = preconditionList.begin();
	list <MyProposition*>::const_iterator itEnd = preconditionList.end();
	for (; it != itEnd; ++it){
		liftedSMTProblem->startNewClause();
		liftedSMTProblem->addPartialActionToClause(partialAction, significantTimePoint, false);
		liftedSMTProblem->AddConditionToCluase(*it, partialAction->partialOperator->op->id, significantTimePoint, true);
		liftedSMTProblem->endClause();
	}
}

void LiftedTranslator::findGoalList (const goal *gl, list <const simple_goal *> &returningList){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		returningList.push_back(simple);
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			findGoalList(*it, returningList);
		}
		return;
	}
	CANT_HANDLE("can't handle some goal in findingGoalList");
}


bool LiftedTranslator::solve(){


	//try to solve the problem
	return liftedSMTProblem->solve(goals);
	return true;
}

double LiftedTranslator::getMetricValue(){
	FastEnvironment env (0);
	return liftedSMTProblem->getExpressionValue(current_analysis->the_problem->metric->expr, &env, 0, translatedLength - 1);
}


void LiftedTranslator::extractSolution (ostream &sout){
	int nOperator = myProblem.operators.size();
	for (int i = 0; i < translatedLength - 1; ++i){
		for (int j = 0; j < nOperator; ++j){
			int nPartialOperator = myProblem.operators[j]->partialOperator.size();
			FastEnvironment *env = new FastEnvironment(myProblem.operators[j]->originalOperator->parameters->size());
			for (int k= 0; k < nPartialOperator; ++k){
				int nPartialActions = myProblem.operators[j]->partialOperator[k]->child.size();
				int l;
				for (l = 0; l < nPartialActions; ++l){
					if (liftedSMTProblem->isPartialActionUsed(myProblem.operators[j]->partialOperator[k]->child[l], i)){
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
							(*env)[*paramIt] = (*(myProblem.actions[j][0].valAction->getEnv()))[*paramIt];
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
