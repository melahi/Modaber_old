/*
 * MyProblem.h
 *
 *  Created on: Aug 6, 2013
 *      Author: sadra
 */

#ifndef MYPROBLEM_H_
#define MYPROBLEM_H_


#include "MyAction.h"
#include "MyAtom.h"
#include "MyOperator.h"
#include "MyPartialAction.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"

#include <map>
#include <vector>


using namespace std;
using namespace VAL;
using namespace Inst;


namespace mdbr {

class MyProblem {
private:

	void assignIdToPropositions();
	void assignIdToVariables();
	void assignIdToValues();

	void assignIdtoUnification ();

public:

	void assignIdToPropositions_EStep();
	void assignIdToValues_EStep();

	vector <vector <MyAction *> >actions;
	vector <MyProposition> propositions;
	vector <MyVariable> variables;

	vector <MyOperator *> operators;
	list <MyPartialAction> partialAction;


	int nPropositionIDs;  //The number of variables needed for propositions for each layer in SAT formula
	int nPartialActions;
	int nVariableIDs;
	int nValueIDs;
	int nUnification;

	vector <double> initialValue;  //The initial value for each variable


	MyProblem(): nPropositionIDs(0), nPartialActions(0){}


	void updateInitialLayer ();


	void initializing();

	void liftedInitializing ();

	void reconsiderValues();
	void reconsiderValues_EStep();

	void write(ostream &sout);

	virtual ~MyProblem() {
		int nOperators = operators.size();
		for (int i = 0; i < nOperators; i++){
			delete(operators[i]);
		}
		for (unsigned int i = 0; i < actions.size(); ++i){
			for (unsigned int j = 0; j < actions[i].size(); ++j){
				delete actions[i][j];
			}
		}
	}
};

extern MyProblem myProblem;

} /* namespace mdbr */
#endif /* MYPROBLEM_H_ */
