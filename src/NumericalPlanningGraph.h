/*
 * NumericalPlanningGraph.h
 *
 *  Created on: Jul 8, 2013
 *      Author: sadra
 */

#ifndef NUMERICALPLANNINGGRAPH_H_
#define NUMERICALPLANNINGGRAPH_H_

#include "MyAction.h"
#include "MyAtom.h"

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"


#include <vector>

using namespace std;
using namespace VAL;


namespace mdbr{

class NumericalPlanningGraph {
public:

	int numberOfLayers;
	int numberOfDynamicMutexesInLastLayer;

	NumericalPlanningGraph();

	void createInitialLayer();

	bool extendOneLayer();

	void write(ostream &sout);

	virtual ~NumericalPlanningGraph();
};

} /* namespace mdbr */

#endif /* NUMERICALPLANNINGGRAPH_H_ */
