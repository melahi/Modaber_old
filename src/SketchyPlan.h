
#ifndef SKETCHYPLAN_H_
#define SKETCHYPLAN_H_


#include "VALfiles/parsing/ptree.h"
#include "NumericRPG.h"
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace VAL;
using namespace std;
using namespace boost;


class SketchyPlan {
	/*
	 * This class in equivalent to chromosome term in Evolutionary Algorithm
	 * and every element intermediateGoals vector is equivalent to a Gene in EA
	 */
private:
	NumericRPG *numericRPG;
	double propositionSelectionRatio;
	int length;
public:

	double fitness;

	vector < vector < shared_ptr <goal> > > milestones;



	SketchyPlan(NumericRPG *numericRPG, int length);

	void createRandomIntermediateGoalLayer (int layerNumber);

	void createRandomSketchyPlan(int length);

	void increaseOneLayer ();

	SketchyPlan crossover(SketchyPlan *mother);

	SketchyPlan mutate();

	void print();

	virtual ~SketchyPlan();

	bool operator < (const SketchyPlan &a) const{
		return fitness > a.fitness;
	}
};

#endif /* SKETCHYPLAN_H_ */
