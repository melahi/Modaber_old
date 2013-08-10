/*
 * StaticMutexFinder.h
 *
 *  Created on: Aug 6, 2013
 *      Author: sadra
 */

#ifndef PRECONDITIONFINDER_H_
#define PRECONDITIONFINDER_H_

#include "VALfiles/FastEnvironment.h"

using namespace VAL;

namespace mdbr {

class PreconditionFinder {
private:

	FastEnvironment *env;
	MyAction *myAction;

public:

	PreconditionFinder (FastEnvironment *env, MyAction *myAction): env(env), myAction(myAction){}

	void simpleGoalAnalyzer(const proposition *prop);

	void expressionAnalyzer (const expression *expr);

	void operator() (const goal *gl);

	virtual ~PreconditionFinder();
};

} /* namespace mdbr */
#endif /* PRECONDITIONFINDER_H_ */
