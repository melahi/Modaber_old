
#ifndef SKETCHYPLAN_H_
#define SKETCHYPLAN_H_


#include "VALfiles/parsing/ptree.h"

using namespace VAL;

class SketchyPlan {
	/*
	 * This class in equivalent to chromosome term in Evolutionary Algorithm
	 * and every element intermediateGoals vector is equivalent to a Gene in EA
	 */
public:

	double fitness;
	goal_list intermediateGoals;


	SketchyPlan();
	void createRandomSketchyPlan();
	void calculateFitness();
	SketchyPlan crossover(SketchyPlan *mother);
	SketchyPlan mutation();
	virtual ~SketchyPlan();
};

#endif /* SKETCHYPLAN_H_ */
