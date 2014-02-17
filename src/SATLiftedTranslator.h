/*
 * LiftedTranslator.h
 *
 *  Created on: Sep 28, 2013
 *      Author: sadra
 */

#ifndef SATLIFTEDTRANSLATOR_H_
#define SATLIFTEDTRANSLATOR_H_

#include "SATSolver.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"
#include "MyAtom.h"
#include "MyPartialAction.h"
#include "LiftedCVC4Problem.h"
#include <vector>

using namespace std;
using namespace VAL;

extern "C" {
	#include "Lingeling/lglib.h"
}

namespace mdbr {

class SATLiftedTranslator {
private:
	SATSolver solver;
	void prepareGoals();
	void addInitialState();
	void addGoals(int significantTimePoint);
	void addPartialActions(int significantTimePoint);
	void addExplanatoryAxioms(int significantTimePoint);
	void addCompletingAction (int significantTimePoint);
	void addAtomMutex(int significantTimePoint);
	void addMetricFunction (int significantTimePoint, MyPartialAction *metricFunction);

public:
	int translatedLength;

	SATLiftedTranslator() {
		solver.prepare(myProblem.nPropositionIDs, myProblem.nUnification, myProblem.nPartialActions, myProblem.nValueIDs);
		solver.refreshLGL();
		solver.prepareTrueValue();
		addInitialState();
		translatedLength = 1;
	}
	void prepare (int length, MyPartialAction *metricFunction);

	bool solve ();

	void getSolution(vector <pair <operator_ *, FastEnvironment> > &solution);

	void extractSolution (ostream &cout);

	void insertSolutionToSMTFormula (LiftedCVC4Problem *smtProblem);




	virtual ~SATLiftedTranslator(){}

private:

	void addSimpleEffectList (polarity plrty, const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction);

	void addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyPartialAction *partialAction = NULL);

	void addGoalList (const list <MyProposition *> &simpleEffectList, int significantTimePoint, MyPartialAction *partialAction);
//	void findGoalList (const goal *gl, list <const simple_goal *> &returningList);

	void addUnacceptablePreconditionBoundaries (int significantTimePoint, MyPartialAction *partialAction);
	void addAssignmentBoundaries (int significantTimePoint, MyPartialAction *partialAction);
};

} /* namespace mdbr */
#endif /* SATLIFTEDTRANSLATOR_H_ */
