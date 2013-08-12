/*
 * SimpleModaber.h
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#ifndef SIMPLEMODABER_H_
#define SIMPLEMODABER_H_

#include "Modaber.h"
#include "CVC4Problem.h"
#include "Translator.h"

namespace mdbr {

class SimpleModaber: public Modaber {
private:

	int nSignificantTimePoint;

	Translator *myTranslator;

	CVC4Problem *smtProblem;

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);

	virtual bool tryToSolve();

public:
	SimpleModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);
	virtual ~SimpleModaber();
};

} /* namespace mdbr */

#endif /* SIMPLEMODABER_H_ */
