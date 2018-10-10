/*
 * Agent.h
 *
 *  Created on: Aug 26, 2013
 *      Author: sadra
 */

#ifndef AGENT_H_
#define AGENT_H_

#include "MyAtom.h"
#include "MyProblem.h"

#include <memory>
#include <vector>



using namespace std;

namespace mdbr {

class Agent {
private:

	bool buildingSolution(int varibleId, int layerNumber);

public:

	int length;

	int fitness;

	int nStateVariables;

	vector < vector < MyStateValue *> > stateValues;   //The first index determine state-variable and second index determine layer number


	Agent():
		  length (0)
		, fitness(-1)
		, nStateVariables(myProblem.stateVariables.size())
		, stateValues (nStateVariables){}

	void increaseLength (int newLength);

	bool buildingSolution ();

	//In the following function, founded state values are converted to goals
	void convertStateValuesToMilestones (vector < vector < shared_ptr <goal> > > &milestones);

	int evaluateFitness ();

	virtual ~Agent ();
};

} /* namespace mdbr */
#endif /* AGENT_H_ */
