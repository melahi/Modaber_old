
#ifndef SKETCHYPLAN_H_
#define SKETCHYPLAN_H_


#include "VALfiles/parsing/ptree.h"
#include "MyStateVariable.h"
#include <vector>
#include <memory>

using namespace VAL;
using namespace std;

using namespace mdbr;


class SketchyPlan {
	/*
	 * This class in equivalent to chromosome term in Evolutionary Algorithm
	 * and every element of intermediateGoals vector is equivalent to a Gene in EA
	 */
private:
	int length;

public:

	double fitness;

	vector < vector < MyStateValue *> > stateValues;   //The first index determine state-variable and second index determine layer number
	int nStateVariables;



	SketchyPlan(int length);

	SketchyPlan();

	void createRandomSketchyPlan(int length);

	//In the buildingWalk function, a walk from Domain Transition Graph of a state variable is built
	void buildingWalk (int variableId, int layerNumber);

	void increaseOneLayer();

	//In the following function founded state values are converted to goals
	void convertStateValuesToMilestones(vector < vector < shared_ptr <goal> > > &milestones);

	//The return pointer of following function should be deleted
	goal* convertPropositionToGoal (const proposition *originalProposition, polarity plrty);

	SketchyPlan crossover(const SketchyPlan *mother) const;

	SketchyPlan mutate() const;

	void write(ostream &sout);

	virtual ~SketchyPlan();

	bool operator < (const SketchyPlan &a) const{
		return fitness > a.fitness;
	}
};

#endif /* SKETCHYPLAN_H_ */
