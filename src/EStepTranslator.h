/*
 * EStepTranslator.h
 *
 *  Created on: Feb 15, 2014
 *      Author: sadra
 */

#ifndef ESTEPTRANSLATOR_H_
#define ESTEPTRANSLATOR_H_

#include "MyProblem.h"
#include "SATSolver.h"
#include "MyAction.h"
#include "VALfiles/instantiation.h"

namespace mdbr {

class EStepTranslator {
private:
	SATSolver solver;
	void prepareGoals();
	void addInitialState();
	void addGoals(int significantTimePoint);
	void addActions(int significantTimePoint);
	void addExplanatoryAxioms(int significantTimePoint);
	void addAtomMutex(int significantTimePoint);
	void addMetricFunction (int significantTimePoint, MyAction *metricFunction);

public:
	int translatedLength;

	EStepTranslator() {
		solver.prepare(myProblem.nPropositionIDs, 0, Inst::instantiatedOp::howMany() , myProblem.nValueIDs);
		solver.refreshLGL();
		solver.prepareTrueValue();
		addInitialState();
		translatedLength = 1;
	}
	void prepare (int length, MyAction *metricFunction);

	bool solve ();

	void getSolution(vector <pair <operator_ *, FastEnvironment> > &solution);

	void extractSolution (ostream &cout);

	virtual ~EStepTranslator(){}

private:

	void addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, MyAction *action = NULL);

	void addPropositionList (bool polarity, int significantTimePoint, list <MyProposition*>::const_iterator it, list <MyProposition*>::const_iterator itEnd, int actionId, int operatorId);
//	void findGoalList (const goal *gl, list <const simple_goal *> &returningList);

	void addUnacceptablePreconditionBoundaries (int significantTimePoint, MyAction *action);
	void addAssignmentBoundaries (int significantTimePoint, MyAction *action);

};

} /* namespace mdbr */
#endif /* ESTEPTRANSLATOR_H_ */
