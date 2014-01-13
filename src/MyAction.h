
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

class MyAction;

class MyAction {
private:

//	bool isPreconditionSatisfied(goal *precondition, FastEnvironment *env, int layerNumber);

	void findPrecondition (const goal *gl);


public:

	instantiatedOp *valAction;

	int firstVisitedLayer;
	bool possibleEffective;

	map <MyProposition*, int > lastLayerPropositionMutexivity;

	list < MyProposition *> deleteList;
	list < MyProposition *> preconditionList;

	void initialize (instantiatedOp *valAction);

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
