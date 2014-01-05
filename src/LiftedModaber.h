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

namespace mdbr {

class LiftedModaber: public Modaber {
private:

	LiftedTranslator *myLiftedTranslator;

	LiftedCVC4Problem *liftedSMTProblem;


	int nSignificantTimePoints;

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve();

public:
	LiftedModaber(char *domainFilePath, char *problemFilePath);
	virtual ~LiftedModaber();
};

} /* namespace mdbr */

#endif /* LIFTEDMODABER_H_ */
