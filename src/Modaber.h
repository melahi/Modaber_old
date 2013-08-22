/* In the name of God  */

#ifndef MODABER_H_
#define MODABER_H_


#include "CVC4Problem.h"
#include "NumericalPlanningGraph.h"
#include "Translator.h"
#include <ostream>

using namespace std;

namespace mdbr{

class Modaber {
protected:

	Translator *myTranslator;

	CVC4Problem *smtProblem;

	NumericalPlanningGraph *nPG;

	bool usingPlanningGraph;

	void instantiation(char *domainFile, char *problemFile);

	virtual void initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph, bool usingSASPlus);

	virtual bool tryToSolve() = 0;

	void extractSolution(ostream &oss, CVC4Problem *smtProblem);


public:

	Modaber();

	virtual ~Modaber();

};

} /* namespace mdbr */

#endif /* MODABER_H_ */
