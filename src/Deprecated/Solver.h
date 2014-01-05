

#ifndef SOLVER_H_
#define SOLVER_H_


#include "Utilities.h"

extern "C" {
	#include "Lingeling/lglib.h"
}

#include "VALfiles/parsing/ptree.h"
#include <iostream>
using namespace std;

using namespace VAL;

namespace mdbr {

class Solver {
private:

	int nOperators;
	int nUnifications;
	int nPartialActions;
	int nPropositions;
	int nAssignments;
	int nComparisons;
	int nValues;
	int nSatVariableForOneLayer;
	LGL *lgl;

	/*
	 * literalId == 0 is reserved for end clause
	 * literalId == 1 is reserved for true
	 * for flase we can use literalId == -1
	 */

	const int trueValueId;
	const int falseValueId;


public:

	int getIdOfOperator (int operatorId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + operatorId;
	}

	int getIdOfUnification (int unificationId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nOperators + unificationId;
	}

	int getIdOfPartialAction (int partialActionId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nOperators + nUnifications + partialActionId;
	}

	int getIdOfProposition (int propositionId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nOperators + nUnifications + nPartialActions + propositionId;
	}

	int getIdOfAssignment (int assignmentId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nOperators + nUnifications + nPartialActions + nPropositions + assignmentId;
	}

	int getIdOfComparison (int comparisonId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nOperators + nUnifications + nPartialActions + nPropositions + nAssignments + comparisonId;
	}

	int getIdOfValue (int valueId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nOperators + nUnifications + nPartialActions + nPropositions + nAssignments + nComparisons + valueId;
	}

	void addLiteral (polarity plrty, int literalId){
		int sign = 1;
		if (plrty == E_NEG){
			sign = -1;
		}
//		cout << sign * literalId << '(' << sign * (((literalId - 2) % nSatVariableForOneLayer) + 2) << ") ";
		lgladd (lgl, sign * literalId);
	}



	Solver():trueValueId(1), falseValueId(-1) {}

	void prepare (int nOperators, int nPropositions, int nUnifications, int nPartialActions, int nAssignments, int nComparisons, int nValues){
		this->nOperators = nOperators;
		this->nPropositions = nPropositions;
		this->nUnifications = nUnifications;
		this->nPartialActions = nPartialActions;
		this->nAssignments = nAssignments;
		this->nComparisons = nComparisons;
		this->nValues = nValues;
		this->nSatVariableForOneLayer = (nPropositions + nOperators + nUnifications + nPartialActions + nAssignments + nComparisons + nValues);
		lgl = 0;
	}

	void refreshLGL () { if (lgl) lglrelease(lgl); lgl = lglinit();}

	void prepareTrueValue(){
		lgladd(lgl, trueValueId);
		lgladd(lgl, 0);
	}
	void endClause() {
//		cout << endl;
		lgladd(lgl, 0);
	}

	void addOperators ( polarity plrty, int operatorId, int significantTimePoint){
		addLiteral(plrty, getIdOfOperator(operatorId, significantTimePoint));
	}
	void addUnification ( polarity plrty, int unificationId, int significantTimePoint){
		addLiteral(plrty, getIdOfUnification(unificationId, significantTimePoint));
	}
	void addPartialAction ( polarity plrty, int partialActionId, int significantTimePoint){
		addLiteral(plrty, getIdOfPartialAction(partialActionId, significantTimePoint));
	}
	void addComparison ( polarity plrty, int comparisonId, int significantTimePoint){
		addLiteral(plrty, getIdOfComparison(comparisonId, significantTimePoint));
	}
	void addAssignment ( polarity plrty, int assignmentId, int significantTimePoint){
		addLiteral(plrty, getIdOfAssignment(assignmentId, significantTimePoint));
	}
	void addValue ( polarity plrty, int valueId, int significantTimePoint){
		addLiteral(plrty, getIdOfValue(valueId, significantTimePoint));
	}

	void addProposition ( polarity plrty, int propositionId, int significantTimePoint){
		if (propositionId < 0){
			if (propositionId == -2){
				addLiteral(plrty, falseValueId);
				return;
			}else{
				CANT_HANDLE("THERE IS SOME UNMEANING PROPOSITION ID: " << propositionId);
			}
		}
		addLiteral(plrty, getIdOfProposition(propositionId, significantTimePoint));
	}

	bool solving() {
		int result = lglsat(lgl);
		if (result == 10){
			return true;
		}
		return false;
	}

	bool isTrueOperator (int operatorId, int significantTimePoint){
		return (lglderef(lgl, getIdOfOperator(operatorId, significantTimePoint)) > 0);
	}

	bool isTrueUnification (int unificationId, int significantTimePoint){
		return (lglderef(lgl, getIdOfUnification(unificationId, significantTimePoint)) > 0);
	}

	virtual ~Solver();
};

} /* namespace mdbr */
#endif /* SOLVER_H_ */
