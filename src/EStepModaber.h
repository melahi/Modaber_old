
#ifndef ESTEPMODABER_H_
#define ESTEPMODABER_H_

#include "Modaber.h"
#include "PlanningGraph.h"
#include "SolutionSimulator.h"
#include "EStepTranslator.h"
#include "MyAction.h"

namespace mdbr {

class EStepModaber: public Modaber {
private:

	int nSignificantTimePoints;

	PlanningGraph *planGraph;

	EStepTranslator *translator;

	SolutionSimulator simulator;

	MyAction metricFunction;
	instantiatedOp *valAction;
	operator_ *valOperator;
	float_expression *metricBound;
	FastEnvironment *env;

	void createMetricFunction();

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve() {return false;}
	virtual bool tryToSolve(double bound);

public:
	EStepModaber (char *domainFilePath, char *problemFilePath, char *solutionFilePath);
	virtual ~EStepModaber();

};

} /* namespace mdbr */
#endif /* ESTEPMODABER_H_ */
