/*
 * LiftedTranslator.h
 *
 *  Created on: Sep 28, 2013
 *      Author: sadra
 */

#ifndef LIFTEDTRANSLATOR_H_
#define LIFTEDTRANSLATOR_H_

#include "Solver.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"
#include "MyLiftedProposition.h"
#include "MyAtom.h"

#include <vector>

using namespace std;
using namespace VAL;

extern "C" {
	#include "Lingeling/lglib.h"
}

namespace mdbr {

class LiftedTranslator {
private:
	Solver solver;

	int nCompletedSignificantTimePoint;

	void addGoal (const goal *gl, int operatorIndex, int significantTimePoint);

	void findIdOfProposition (MyLiftedProposition &theProposition, int operatorIndex, int &id, int &significantTimePoint);
	void findIdOfValue (MyValue &theValue, int operatorIndex, int &id, int &significantTimePoint);

public:
	LiftedTranslator();

	void prepare (int nOperators, int nPropositions, int nUnifications, int nPartialActions, int nAssignments, int nComparisons, int nValues) {
		solver.prepare (nOperators, nPropositions, nUnifications, nPartialActions, nAssignments, nComparisons, nValues);
	}

	void addInitialState();

	void addGoals(int significantTimePoint);

	void addOperators(int significantTimePoint);

	void addPartialActions (int significantTimePoint);

	void addComparisons (int significantTimePoint);

	void addAssignments (int significantTimePoint);

	void addExplanatoryAxioms(int significantTimePoint);

	void addAtomMutex(int significantTimePoint);

	void buildFormula (int nSignificantTimePoints);

	bool solve (int nSignificantTimePoints);

	void printSolution (ostream &sout);

	void getSolution (vector <pair <operator_ *, FastEnvironment> > &solution);

	void writeSATProposition (ostream &sout);

	virtual ~LiftedTranslator();
};

} /* namespace mdbr */
#endif /* LIFTEDTRANSLATOR_H_ */
