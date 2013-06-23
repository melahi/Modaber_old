/* In the name of God  */

#ifndef MODABER_H_
#define MODABER_H_


#include "CVC4Problem.h"
#include "NumericRPG.h"
#include "MyAnalyzer.h"
#include "SketchyPlan.h"
#include <ostream>

using namespace std;

class Modaber {
protected:

	MyAnalyzer *myAnalyzer;
	NumericRPG *numericRPG;

	void instantiation(char *domainFile, char *problemFile);

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve() = 0;

	void extractSolution(ostream &oss, CVC4Problem *smtProblem);


public:

	Modaber();

	virtual ~Modaber();

};

#endif /* MODABER_H_ */
