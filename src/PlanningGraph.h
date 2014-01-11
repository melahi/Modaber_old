
#ifndef PLANNINGGRAPH_H_
#define PLANNINGGRAPH_H_

#include "MyAction.h"
#include "MyAtom.h"

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"


#include <vector>
#include <limits>

using namespace std;
using namespace VAL;


namespace mdbr{

class PlanningGraph {
public:

	int numberOfLayers;
	int numberOfDynamicMutexesInLastLayer;
	bool levelOff;


	PlanningGraph();

	void ignoreGraph();

	void constructingGraph (int maximumLayerNumber = numeric_limits <int>::max());

	void createInitialLayer();

	bool extendOneLayer();

	virtual ~PlanningGraph();
};

} /* namespace mdbr */

#endif /* PLANNINGGRAPH_H_ */
