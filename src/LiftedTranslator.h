
#ifndef LIFTEDTRANSLATOR_H_
#define LIFTEDTRANSLATOR_H_


#include "LiftedCVC4Problem.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "MyTimer.h"
#include <ptree.h>
#include "Utilities.h"

using namespace VAL;
using namespace Inst;







class LiftedTranslator {
private:
	LiftedCVC4Problem *liftedSMTProblem;
	Expr goals;
	void prepareGoals();
	void addInitialState();
	void addGoals(int significantTimePoint);
	void addPartialActions(int significantTimePoint);
	void addExplanatoryAxioms(int significantTimePoint);
	void addCompletingAction (int significantTimePoint);
	void addMetric (double bound, int significantTimePoint);

public:
	int translatedLength;

	LiftedTranslator(LiftedCVC4Problem* liftedSMTProblem) :
		liftedSMTProblem(liftedSMTProblem) {
		this->liftedSMTProblem->activePermanentChange();
		addInitialState();
		this->liftedSMTProblem->inActivePermanentChange();
		translatedLength = 1;
	}
	void prepare (int length);

	bool solve ();

	void extractSolution (ostream &cout);

	virtual ~LiftedTranslator(){}

private:

	void addSimpleEffectList (polarity plrty, const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction);

	void addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction = NULL);
	void addAssignmentList (const list <const assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction);

	void addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction = NULL);

	void addGoalList (const list <const comparison* > &gls, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction);
	void addGoalList (const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction);
	void findGoalList (const goal *gl, list <const simple_goal *> &returningList);

};

#endif /* LIFTEDTRANSLATOR_H_ */
