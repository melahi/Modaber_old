#ifndef NUMERICRPG_H_
#define NUMERICRPG_H_

#include <vector>
#include <iostream>
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"

using namespace std;
using namespace VAL;

class NumericRPG {
private:

	/*
	 * RPG will be extended one layer by calling extendOneLayer function.
	 * The return value of this function determine terminate condition for extending RPG.
	 */
	bool extendOneLayer();

	void constructFirstLayer();

	bool isApplicable (int actionId);

	bool isPreconditionSatisfied(goal *precondition, FastEnvironment *env);

	/*
	 * The following function estimate the range of an expression from minimum value to its possible maximum value
	 * the minimumValue and maximumValue variables are return values
	 */
	void estimateExpression (const expression *expr, FastEnvironment *env, double &minimumValue, double &maximumValue);

	void applyAction (int actionId);

	void addSimpleEffectList (const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env);

	void addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env);


public:

	vector <int> firstVisitedProposition;
	vector <int> firstVisitedAcotion;
	vector < vector <double> > minPNEValue;   //The first index corresponds to layer number and second index corresponds to PNE index
	vector < vector <double> > maxPNEValue;   //The first index corresponds to layer number and second index corresponds to PNE index
	int numberOfLayers;
	int minimumPlanLength;



	NumericRPG();

	int findMinimumLevelSatisfyingGoal (goal *gl, FastEnvironment *env);

	void print(ostream &sout);

	virtual ~NumericRPG();
};

#endif /* NUMERICRPG_H_ */
