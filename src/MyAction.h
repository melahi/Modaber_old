
#ifndef MYACTION_H_
#define MYACTION_H_

#include <list>
#include <set>
#include <map>
#include <iostream>
#include "MyAtom.h"
#include "MyAction.h"
#include "Utilities.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"


using namespace std;
using namespace Inst;
using namespace VAL;



namespace mdbr{


class MyAtom;
class   MyProposition;
class   MyVariable;
class MyBound;

class MyAction;

class MyAction {
private:

//	bool isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber);

	void findPrecondition (const goal *gl);


	void findingVariablesMinimum (const expression *expr, map <int, pair <bool, bool> > &result);
	void findingVariablesMaximum (const expression *expr, map <int, pair <bool, bool> > &result);

	double evalMinimum (const expression *expr, map <int, pair <double, double> > &selectedValue);
	double evalMaximum (const expression *expr, map <int, pair <double, double> > &selectedValue);

	void createMyBoundVector (map <int, pair<double, double> > &values, vector <MyBound> &result);

	void constructComparisons (const goal *gl);
	void constructComparisons (const comparison *expr);
	void constructComparisons (const comparison *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values);

	void constructAssignments ();
	void constructAssignmentsMaximum (const assignment *expr);
	void constructAssignmentsMinimum (const assignment *expr);
	void constructAssignmentsMaximum (const assignment *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values);
	void constructAssignmentsMinimum (const assignment *expr, map <int, pair <bool, bool> > &variables, map <int, pair <bool, bool> >::iterator it, map <int, pair <double, double> > &values);

public:

	int id;

	instantiatedOp *valAction;

	int firstVisitedLayer;
	bool possibleEffective;

	map <MyProposition*, int > lastLayerPropositionMutexivity;

	list < MyProposition *> deleteList;
	list < MyProposition *> addList;
	list < MyProposition *> preconditionList;


	vector < vector <MyBound> > unacceptablePreconditionBoundaries;
	vector < pair < vector <MyBound>, MyBound > > assignmentBoundaries;


	void initialize (instantiatedOp *valAction, int id);

	void constructNumericalCondition();

	bool isPropositionStaticallyMutex (MyProposition *otherProposition);

	bool isApplicable (int layerNumber);

	void applyAction (int layerNumber);
	void addSimpleEffectList (const pc_list <simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber);


	bool isPropositionDynamicallyMutex (int layerNumber, MyProposition *otherProposition);
	bool isPropositionMutex (int layerNumber, MyProposition *otherProposition);
	bool checkDynamicPropositionMutex (int layerNumber, MyProposition *otherProposition);

	void insertPropositionMutex (int layerNumber, MyProposition *otherProposition);

	void write (ostream &sout);

	MyAction();
	virtual ~MyAction();
};

} /* namespace mdbr */

#endif /* MYACTION_H_ */
