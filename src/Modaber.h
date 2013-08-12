/* In the name of God  */

#ifndef MODABER_H_
#define MODABER_H_


#include "CVC4Problem.h"
#include "NumericalPlanningGraph.h"
#include <ostream>

using namespace std;

namespace mdbr{

class Modaber {
protected:

	NumericalPlanningGraph *nPG;

	bool usingPlanningGraph;

	void instantiation(char *domainFile, char *problemFile);

	virtual void initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);

	virtual bool tryToSolve() = 0;

	void extractSolution(ostream &oss, CVC4Problem *smtProblem);


public:

	Modaber();

	virtual ~Modaber();

};

} /* namespace mdbr */

#endif /* MODABER_H_ */
