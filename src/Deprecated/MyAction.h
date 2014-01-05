
#ifndef MYACTION_H_
#define MYACTION_H_

#include <list>
#include <set>
#include <map>
#include <iostream>
#include "MyAtom.h"
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
class   MyValue;
class MyVariable;

class MyAction;
class MyGroundedAction;

class MyAction {
private:

	bool isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber);


public:

	instantiatedOp *valAction;

	int firstVisitedLayer;

	map <MyAction*, int > lastLayerMutexivity;
	map <MyProposition*, int > lastLayerPropositionMutexivity;



	set < MyAction * > staticMutex;

	//deleteList and variableModifier are used for finding static propositional mutex so we are keeping them in set container
	set < MyProposition *> deleteList;
	set < MyVariable *> modifyingVariable;

	//Perhaps we need a variable more than one in some precondition so we should consider variableNeeded as a set
	set < MyVariable *> variableNeeded;

	list < MyProposition *> addList;
	list < MyProposition *> propositionPrecondition;

	void initialize (instantiatedOp *valAction);

	void computeStaticMutex();
	bool isStaticallyMutex(MyAction *otherAction);
	bool isPropositionStaticallyMutex (MyProposition *otherProposition);

	bool isApplicable (int layerNumber);

	void applyAction (int layerNumber);
	void addSimpleEffectList (const pc_list <simple_effect*> &simpleEffectList, FastEnvironment *env, int layerNumber);




	bool isDynamicallyMutex(int layerNumber, MyAction *otherAction);
	bool isMutex (int layerNumber, MyAction *otherAction);

	bool isPropositionDynamicallyMutex (int layerNumber, MyProposition *otherProposition);
	bool isPropositionMutex (int layerNumber, MyProposition *otherProposition);

	bool checkDynamicMutex (int layerNumber, MyAction *otherAction);
	bool checkDynamicPropositionMutex (int layerNumber, MyProposition *otherProposition);

	void insertMutex (int layerNumber, MyAction *otherAciton);
	void insertPropositionMutex (int layerNumber, MyProposition *otherProposition);

	void write (ostream &sout);

	MyAction();
	virtual ~MyAction();
};

} /* namespace mdbr */

#endif /* MYACTION_H_ */
