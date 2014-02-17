
#ifndef SATRANGEDMODABER_H_
#define SATRANGEDMODABER_H_

#include "Modaber.h"
#include "LiftedTranslator.h"
#include "SolutionSimulator.h"
#include "PlanningGraph.h"
#include "SATLiftedTranslator.h"
#include "MyPartialAction.h"

namespace mdbr {

class SatRangedModaber: public Modaber {
private:

	int nSignificantTimePoints;

	PlanningGraph *planGraph;

	SATLiftedTranslator *translator;

	SolutionSimulator simulator;

	MyPartialOperator metricFunctionOperator;
	MyPartialAction metricFunction;
	float_expression *metricBound;
	FastEnvironment *env;

	void createMetricFunction();

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve() {return false;}
	virtual bool tryToSolve(double bound);

public:
	SatRangedModaber(char *domainFilePath, char *problemFilePath, char *solutionFilePath);
	virtual ~SatRangedModaber();
};

} /* namespace mdbr */

#endif /* SATRANGEDMODABER_H_ */
