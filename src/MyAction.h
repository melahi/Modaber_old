
#ifndef MYACTION_H_
#define MYACTION_H_

#include <list>
#include <set>
#include <map>
#include <iostream>
#include "MyAtom.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"


using namespace std;
using namespace Inst;
using namespace VAL;



namespace mdbr{


class MyAtom;

class MyAction {
public:

	instantiatedOp *valAction;

	int firstVisitedLayer;


	set < MyAction * > staticMutex;

	//deleteList and variableModifier are used for finding static propositional mutex so we are keeping them in set container
	set < MyProposition *> deleteList;
	set < MyVariable *> modifyingVariable;

	list < MyProposition *> addList;
	list < MyVariable *> variableNeeded;
	list < MyProposition *> propositionPrecondition;

	void initialize (instantiatedOp *valAction);

	void computeStaticMutex();
	bool isStaticallyMutex(int layerNumber, MyAction *otherAction);
	bool isAtomStaticallyMutex (int layerNumber, MyAtom *atom);

	list< MyGroundedAction > groundedAction;
	bool computeGroundedAction (int layerNumber);


	MyAction();
	virtual ~MyAction();
};

class MyGroundedAction {
private:

	double evaluateExpression (const expression *expr, FastEnvironment *env);

	bool isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber);

public:

	int firstVisitedLayer;

	map <MyGroundedAction*, int > lastLayerMutexivity;
	map < MyAtom *, int > lastLayerAtomMutexivity;

	map < int, MyValue *> variablePrecondition;  //it's a map from variable id (or PNE id) to an atom (or MyValue *)
	MyAction *parentAction;


	bool isApplicable (int layerNumber);

	void applyAction (int layerNumber);
	void addSimpleEffectList (const pc_list <simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber);
	void addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int layerNumber);


	bool isDynamicallyMutex(int layerNumber, MyGroundedAction *otherAction);
	bool isMutex (int layerNumber, MyGroundedAction *otherAction);

	bool isAtomDynamicallyMutex (int layerNumber, MyAtom *otherAtom);
	bool isAtomMutex (int layerNumber, MyAtom *otherAtom);

	bool checkDynamicMutex (int layerNumber, MyGroundedAction *otherAction);
	bool checkDynamicAtomMutex (int layerNumber, MyAtom *otherAtom);

	void insertMutex (int layerNumber, MyGroundedAction *otherAciton);
	void insertAtomMutex (int layerNumber, MyAtom *otherAtom);

	MyGroundedAction (MyAction *parentAction, const map <int, MyValue*> &variablePrecondition): parentAction(parentAction), variablePrecondition(variablePrecondition) {}

	void write (ostream &sout);

};

} /* namespace mdbr */

#endif /* MYACTION_H_ */
