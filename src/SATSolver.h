

#ifndef SOLVER_H_
#define SOLVER_H_


#include "Utilities.h"

extern "C" {
	#include "Lingeling/lglib.h"
}

#include "MyProblem.h"

#include "VALfiles/parsing/ptree.h"
#include <iostream>
using namespace std;

using namespace VAL;

namespace mdbr {

class SATSolver {
private:

	int nUnifications;
	int nPartialActions;
	int nPropositions;
	int nSatVariableForOneLayer;
	LGL *lgl;

	/*
	 * literalId == 0 is reserved for end clause
	 * literalId == 1 is reserved for true
	 * for flase we can use literalId == -1
	 */

	const int trueValueId;
	const int falseValueId;



	int getIdOfUnification (int unificationId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + unificationId;
	}

	int getIdOfPartialAction (int partialActionId, int significantTimePoint){
		return 2 + significantTimePoint * nSatVariableForOneLayer + nUnifications + partialActionId;
	}

	int getIdOfProposition (int propositionId, unsigned int operatorId, int significantTimePoint){

		if (! myProblem.propositions[propositionId].possibleEffective ){
			return trueValueId;
		}
		if (! isVisited(myProblem.propositions[propositionId].firstVisitedLayer, (myProblem.operators.size() * significantTimePoint + operatorId))){
			return falseValueId;
		}

		if (propositionId == -1){
			return trueValueId;
		}
		if (operatorId == myProblem.operators.size() || myProblem.propositions[propositionId].ids[operatorId] == -1){
			significantTimePoint++;
			operatorId = 0;
		}

		return 2 + significantTimePoint * nSatVariableForOneLayer + nUnifications + nPartialActions + myProblem.propositions[propositionId].ids[operatorId];
	}


	void addLiteral (int literalId, bool plrty){
		int sign = 1;
		if (!plrty){
			sign = -1;
		}
//		cout << sign * literalId << '(' << sign * (((literalId - 2) % nSatVariableForOneLayer) + 2) << ") ";
		lgladd (lgl, sign * literalId);
	}



public:


	SATSolver():trueValueId(1), falseValueId(-1) {}

	void prepare (int nPropositions, int nUnifications, int nPartialActions){
		this->nPropositions = nPropositions;
		this->nUnifications = nUnifications;
		this->nPartialActions = nPartialActions;
		this->nSatVariableForOneLayer = (nPropositions + nUnifications + nPartialActions);
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

	void addUnification (int unificationId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfUnification(unificationId, significantTimePoint), plrty);
	}
	void addPartialAction (int partialActionId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfPartialAction(partialActionId, significantTimePoint), plrty);
	}

	void addProposition (int propositionId, int operatorId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfProposition(propositionId, operatorId, significantTimePoint), plrty);
	}

	bool solving() {
		int result = lglsat(lgl);
		if (result == 10){
			return true;
		}
		return false;
	}

	bool isTruePartialAction (int partialActionId, int significantTimePoint){
		return (lglderef(lgl, getIdOfPartialAction(partialActionId, significantTimePoint)) > 0);
	}

	virtual ~SATSolver();
};

} /* namespace mdbr */
#endif /* SOLVER_H_ */
