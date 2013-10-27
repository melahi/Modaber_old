/*
 * SimpleModaber.h
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#ifndef LIFTEDMODABER_H_
#define LIFTEDMODABER_H_

#include "Modaber.h"
#include "LiftedTranslator.h"
#include "SolutionSimulator.h"

namespace mdbr {

class LiftedModaber: public Modaber {
private:

	int nSignificantTimePoints;

	LiftedTranslator translator;

	SolutionSimulator simulator;

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);

	virtual bool tryToSolve();

public:
	LiftedModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);
	virtual ~LiftedModaber();
};

} /* namespace mdbr */

#endif /* LIFTEDMODABER_H_ */
