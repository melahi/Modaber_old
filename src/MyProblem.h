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
#include "MyStateVariable.h"
#include "MyEnvironment.h"
#include "MyObject.h"
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
	void readingSASPlusFile();
	void buildingDTG();


	void filterVariables();

	void assignIdToPropositions();
	void assignIdToVariables();

public:

	bool usingSASPlus;

	vector <MyAction> actions;
	vector <MyProposition> propositions;
	vector <MyVariable> variables;
	vector <MyStateVariable> stateVariables;

	map <VAL::pddl_type *, MyType> types;
	map <VAL::const_symbol *, MyObject> objects;
	vector <MyOperator *> operators;
	list <MyPartialAction> partialAction;


	int nUnification;
	int nPropositionIDs;  //The number of variables needed for propositions for each layer in SAT formula
	int nPartialActions;
	int nVariableIDs;

	MyEnvironment environment;

	vector <double> initialValue;  //The initial value for each variable
	vector <MyStateValue*> goalValue;


	MyProblem():usingSASPlus(false), nUnification(0), nPropositionIDs(0), nPartialActions(0){}


	void updateInitialLayer ();

	void updateGoalValues();

	void updateGoalValues (goal *the_goal, FastEnvironment *env);

	void initializing(bool usingSASPlus);

	void liftedInitializing ();

	void write(ostream &sout);

	void writeType (ostream &sout, MyType * type, int indent);

	void writeDTG(ostream &sout);

	virtual ~MyProblem() {
		int nOperators = operators.size();
		for (int i = 0; i < nOperators; i++){
			delete(operators[i]);
		}
	}
};

extern MyProblem myProblem;

} /* namespace mdbr */
#endif /* MYPROBLEM_H_ */
