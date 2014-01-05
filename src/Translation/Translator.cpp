//In the name of God


#include "Translator.h"


/*
void Translator::addInitialState(){
	bool *proposition = new bool [pProblem.pAllProposition.size()];

	for (unsigned int i = 0; i < pProblem.pAllProposition.size(); i++){
		proposition[i] = false;
	}

	for (unsigned int i = 0; i < pProblem.initialState.size(); i++){
		proposition[i] = true;
	}

	for (unsigned int i = 0; i < pProblem.pAllProposition.size(); i++){
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(i, 0, proposition[i]);
		smtProblem->endClause();
	}
}

void Translator::addGoals(int significantTimePoint){
	for (unsigned int i = 0; i < pProblem.goals.size(); i++){
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(pProblem.goals[i], significantTimePoint, true);
		smtProblem->endClause();
	}
}

inline void Translator::addPropositionalActionPreconditions (int actionID, int significantTimePoint){
	set <int>::const_iterator it;
	set <int>::const_iterator endIterator = pProblem.pAllAction.all[actionID].conditionAtStart.end();
	for (it = pProblem.pAllAction.all[actionID].conditionAtStart.begin(); it != endIterator; it++){
		int proposition = *it;
		smtProblem->startNewClause();
		smtProblem->addActionToClause(actionID, significantTimePoint, false);
		smtProblem->addConditionToCluase(proposition, significantTimePoint, true);
		smtProblem->endClause();
	}
}

inline void Translator::addNumericalActionPreconditions (int actionID, int significantTimePoint){
	vector <PComparisonExpression *>::const_iterator it;
	vector <PComparisonExpression *>::const_iterator endIterator = pProblem.pAllAction.all[actionID].comparisonAtStart.end();
	for (it = pProblem.pAllAction.all[actionID].comparisonAtStart.begin(); it != endIterator; it++){
		PComparisonExpression *comparison = *it;
		smtProblem->startNewClause();
		smtProblem->addActionToClause(actionID, significantTimePoint, false);
		smtProblem->addConditionToCluase(comparison, significantTimePoint, true);
		smtProblem->endClause();
	}
}


inline void Translator::addPropositionalActionEffects (int actionID, int significantTimePoint){
	set <int>::const_iterator it;

	//Add effects
	set <int>::const_iterator endIterator = pProblem.pAllAction.all[actionID].addEffectAtEnd.end();
	for (it = pProblem.pAllAction.all[actionID].addEffectAtEnd.begin(); it != endIterator; it++){
		int proposition = *it;
		smtProblem->startNewClause();
		smtProblem->addActionToClause(actionID, significantTimePoint, false);
		smtProblem->addConditionToCluase(proposition, significantTimePoint + 1, true);
		smtProblem->endClause();
	}

	//Delete effects
	endIterator = pProblem.pAllAction.all[actionID].delEffectAtEnd.end();
	for (it = pProblem.pAllAction.all[actionID].delEffectAtEnd.begin(); it != endIterator; it++){
		int proposition = *it;
		smtProblem->startNewClause();
		smtProblem->addActionToClause(actionID, significantTimePoint, false);
		smtProblem->addConditionToCluase(proposition, significantTimePoint + 1, false);
		smtProblem->endClause();
	}

}

inline void Translator::addNumericalActionEffects (int actionID, int significantTimePoint){
	vector <PAssignment *>::const_iterator it;
	vector <PAssignment *>::const_iterator endIterator = pProblem.pAllAction.all[actionID].assignmentAtEnd.end();
	for (it = pProblem.pAllAction.all[actionID].assignmentAtEnd.begin(); it != endIterator; it++){
		PAssignment *assignment = *it;
		smtProblem->startNewClause();
		smtProblem->addActionToClause(actionID, significantTimePoint, false);
		smtProblem->addConditionToCluase(assignment, significantTimePoint + 1, true);
		smtProblem->endClause();
	}
}

void Translator::addActions(int significantTimePoint){
	unsigned actionSize = pProblem.pAllAction.size();
	for (unsigned int i = 0; i < actionSize; i++){
		addPropositionalActionPreconditions(i, significantTimePoint);
		addNumericalActionPreconditions(i, significantTimePoint);
		addPropositionalActionEffects(i, significantTimePoint);
		addNumericalActionEffects(i, significantTimePoint);
	}
}


void Translator::addExplanatoryAxioms (int significantTimePoint){
	if (significantTimePoint < 1){
		return;
	}
	for (unsigned int i = 0; i < pProblem.pAllProposition.size(); i++){
		// p(t) and ~p(t-1) => at least one action with the effect of p(t) is executed on time (t-1)
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(i, significantTimePoint, false);
		smtProblem->addConditionToCluase(i, significantTimePoint - 1, true);
		for (set <int>::const_iterator it = pProblem.pAllProposition.all[i].addEffectAtEnd.begin(); it != pProblem.pAllProposition.all[i].addEffectAtEnd.begin(); ++it){
			smtProblem->addActionToClause(*it, significantTimePoint - 1, true);
		}
		smtProblem->endClause();
	}
	for (unsigned int i = 0; i < pProblem.pAllProposition.size(); i++){
		// ~p(t) and p(t-1) => at least one action with the effect of p(t) is executed on time (t-1)
		smtProblem->startNewClause();
		smtProblem->addConditionToCluase(i, significantTimePoint, true);
		smtProblem->addConditionToCluase(i, significantTimePoint - 1, false);
		for (set <int>::const_iterator it = pProblem.pAllProposition.all[i].delEffectAtEnd.begin(); it != pProblem.pAllProposition.all[i].delEffectAtEnd.begin(); ++it){
			smtProblem->addActionToClause(*it, significantTimePoint - 1, true);
		}
		smtProblem->endClause();
	}
}

void Translator::addActionMutex(int significantTimePoint){
	for (unsigned int i = 0; i < pProblem.pAllAction.size(); i++){
		for (set<int>::const_iterator j = pProblem.pAllAction.all[i].mutexActions.begin(); j != pProblem.pAllAction.all[i].mutexActions.end(); j++){
			smtProblem->startNewClause();
			smtProblem->addActionToClause(i, significantTimePoint, false);
			smtProblem->addActionToClause(*j, significantTimePoint, false);
			smtProblem->endClause();
		}
	}
}

*/



