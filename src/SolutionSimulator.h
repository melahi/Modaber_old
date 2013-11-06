#ifndef SOLUTIONSIMULATOR_H_
#define SOLUTIONSIMULATOR_H_

#include "MyProblem.h"
#include "VALfiles/FastEnvironment.h"
#include "VALfiles/parsing/ptree.h"

#include <vector>
#include <set>

using namespace std;
using namespace VAL;


namespace mdbr {


class State {
public:
	set <int> trueLiterals;
	vector <double> variableValues;
};


class SolutionSimulator {
private:

	void prepareInitialState(State &state);

	double evaluateExpression (const expression *expr, FastEnvironment *env, State &state);

	bool isApplicable (goal *gl, FastEnvironment *env, State &state);

	void apply (operator_ * op, FastEnvironment *env, State &state);
	void applyAddEffect (pc_list<simple_effect*> *addList, FastEnvironment *env, State &state);
	void applyDeleteEffect (pc_list<simple_effect*> *deleteList, FastEnvironment *env, State &state);
	void applyAssignmentList (const pc_list <assignment *> *assignmentEffects, FastEnvironment *env, State &state);




public:
	int countValues();

	SolutionSimulator();
	virtual ~SolutionSimulator();


	bool isValid (vector <pair <operator_ *, FastEnvironment> >  &solution);
};

} /* namespace mdbr */
#endif /* SOLUTIONSIMULATOR_H_ */
