/*
 * SimpleModaber.h
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#ifndef SIMPLEMODABER_H_
#define SIMPLEMODABER_H_

#include "Modaber.h"

class SimpleModaber: public Modaber {
private:

	int nSignificantTimePoint;

	CVC4Problem *smtProblem;

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);

	virtual bool tryToSolve();

public:
	SimpleModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);
	virtual ~SimpleModaber();
};

#endif /* SIMPLEMODABER_H_ */
