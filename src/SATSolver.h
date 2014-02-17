

#ifndef SOLVER_H_
#define SOLVER_H_

//#define PRINT_FORMULA



#include "Utilities.h"

extern "C" {
	#include "Lingeling/lglib.h"
}

#include "MyProblem.h"
#include "MyAtom.h"

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include <iostream>

#ifdef PRINT_FORMULA
#include <map>
#include <string>
#include <sstream>
#endif


using namespace std;

using namespace VAL;

namespace mdbr {

class SATSolver {
private:

#ifdef PRINT_FORMULA
	map <int, string> literalName;
#endif

	int nUnifications;
	int nActions;
	int nPropositions;
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



	int getIdOfUnification (int unificationId, int significantTimePoint){
		int ret = 2 + significantTimePoint * nSatVariableForOneLayer + unificationId;
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[ unification: " << unificationId << ", " << significantTimePoint << " ]";
		literalName [ret] = oss.str();
#endif
		return ret;
	}

	int getIdOfAction (int actionId, int significantTimePoint){
		int ret = 2 + significantTimePoint * nSatVariableForOneLayer + nUnifications + actionId;
#ifdef PRINT_FORMULA
		if (literalName.find(ret) == literalName.end()){
			ostringstream oss;
			oss << "[ Action: ";
			OpStore::iterator it, itEnd;
			it = instantiatedOp::opsBegin();
			itEnd = instantiatedOp::opsEnd();
			for (; it != itEnd; ++it){
				if ((*it)->getID() == actionId){
					(*it)->write(oss);
					break;
				}
			}
			oss << ", " << significantTimePoint << " ]";
			literalName [ret] = oss.str();
		}
#endif
		return ret;
	}

	int getIdOfProposition (int propositionId, unsigned int operatorId, int significantTimePoint){

		if (! myProblem.propositions[propositionId].possibleEffective ){
			return trueValueId;
		}
		int nActions = Inst::instantiatedOp::howMany();
		if (! isVisited(myProblem.propositions[propositionId].firstVisitedLayer, (int)(significantTimePoint * nActions + operatorId))){
			return falseValueId;
		}

		if (propositionId == -1){
			return trueValueId;
		}
		if (operatorId == myProblem.propositions[0].ids.size() || myProblem.propositions[propositionId].ids[operatorId] == -1){
			significantTimePoint++;
			operatorId = 0;
		}

		int ret = 2 + significantTimePoint * nSatVariableForOneLayer + nUnifications + nActions + myProblem.propositions[propositionId].ids[operatorId];
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[ proposition: ";
		myProblem.propositions[propositionId].write(oss);
		oss << ", " << operatorId << ", " << significantTimePoint << ", " << ret << " ]";
		literalName [ret] = oss.str();
#endif
		return ret;
	}

	int getIdOfValue (map <double, vector <int> >::iterator it, boundKind myKind, unsigned int operatorId, int significantTimePoint){
		if (operatorId == it->second.size() || it->second[operatorId] == -1){
			significantTimePoint++;
			operatorId = 0;
		}

		int ret = 2 + significantTimePoint * nSatVariableForOneLayer + nUnifications + nActions + nPropositions + (2 * it->second[operatorId]);
		if (myKind == upperBound){
			++ret;
		}
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[ ";
		if (myKind == upperBound){
			oss << "upper";
		}else{
			oss << "lower";
		}
		oss << "value: ";
		int nVariables = myProblem.variables.size();
		bool needSearching = true;
		for (int i = 0; needSearching && i < nVariables; ++i){
			map <double, vector <int> >::iterator domainIt, domainItEnd;
			FOR_ITERATION(domainIt, domainItEnd, myProblem.variables[i].domain){
				if (domainIt->second[operatorId] == it->second[operatorId]){
					myProblem.variables[i].originalPNE->write(oss);
					needSearching = false;
					break;
				}
			}
		}
		oss << "=" << it->first << ", id: " << ret << ", significantTimePoint: " << significantTimePoint << " ]";
		literalName [ret] = oss.str();
#endif
		return ret;
	}


	void addLiteral (int literalId, bool plrty){
		int sign = 1;
		if (!plrty){
			sign = -1;
		}

#ifdef PRINT_FORMULA
//		cout << sign * literalId << '(' << sign * (((literalId - 2) % nSatVariableForOneLayer) + 2) << ") ";
		cout << "(";
		if (!plrty){
			cout << "NOT ";
		}
		cout << literalName[literalId] << ") ";
#endif
		lgladd (lgl, sign * literalId);
	}



public:


	SATSolver():trueValueId(1), falseValueId(-1) {}

	void prepare (int nPropositions, int nUnifications, int nActions, int nValues){
		this->nPropositions = nPropositions;
		this->nUnifications = nUnifications;
		this->nActions = nActions;
		this->nValues = (nValues * 2);
		this->nSatVariableForOneLayer = (nPropositions + nUnifications + nActions + (nValues * 2));
		lgl = 0;
	}

	void refreshLGL () { if (lgl) lglrelease(lgl); lgl = lglinit();}

	void prepareTrueValue(){
#ifdef PRINT_FORMULA
		literalName[trueValueId] = " [TRUE] ";
		literalName[falseValueId] = " [FALSE] ";
#endif
		lgladd(lgl, trueValueId);
		lgladd(lgl, 0);
	}
	void endClause() {
#ifdef PRINT_FORMULA
		cout << endl;
#endif
		lgladd(lgl, 0);
	}

	void addUnification (int unificationId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfUnification(unificationId, significantTimePoint), plrty);
	}
	void addAction (int actionId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfAction(actionId, significantTimePoint), plrty);
	}

	void addProposition (int propositionId, int operatorId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfProposition(propositionId, operatorId, significantTimePoint), plrty);
	}

	void addValue (map <double, vector<int> >::iterator it, boundKind myKind, int operatorId, int significantTimePoint, bool plrty){
		addLiteral(getIdOfValue(it, myKind, operatorId, significantTimePoint), plrty);
	}

	bool solving() {
		int result = lglsat(lgl);
		if (result == 10){
			return true;
		}
		return false;
	}

	bool isTrueAction (int actionId, int significantTimePoint){
		return (lglderef(lgl, getIdOfAction(actionId, significantTimePoint)) > 0);
	}

	virtual ~SATSolver(){
		lglrelease(lgl);
	}
};

} /* namespace mdbr */
#endif /* SOLVER_H_ */
