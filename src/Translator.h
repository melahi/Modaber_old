
#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_


#include "CVC4Problem.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "SketchyPlan.h"
#include "MyTimer.h"
#include <ptree.h>
#include "Utilities.h"

using namespace VAL;
using namespace Inst;







class Translator {
private:
	CVC4Problem *smtProblem;
	Expr goals;
	void prepareGoals();
	void addInitialState();
	void addGoals(int significantTimePoint);
	void addActions(int significantTimePoint);
	void addExplanatoryAxioms(int significantTimePoint);
	void addActionMutex(int significantTimePoint);
	void addAtomMutex(int significantTimePoint);
	void addSkechyPlan(SketchyPlan* sketchyPlan);

public:
	int translatedLength;

	Translator(CVC4Problem* smtProblem) :
		smtProblem(smtProblem) {
		this->smtProblem->activePermanentChange();
		addInitialState();
		this->smtProblem->inActivePermanentChange();
		translatedLength = 1;
	}
	void prepare (int length);

	double solve (SketchyPlan *sketchyPlan);

	bool solve ();

	virtual ~Translator(){}

private:

	void addSimpleEffectList (polarity plrty, const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env, int significantTimePoint, int actionID = -1);

	void addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, int actionID = -1);

	void addEffectList (const effect_lists *effects, FastEnvironment *env, int significantTimePoint, int actionId = -1);

	void addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, int actionId = -1);


};

#endif /* TRANSLATOR_H_ */
