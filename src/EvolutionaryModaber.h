/*
 * EvolutionaryModaber.h
 *
 *  Created on: Jun 11, 2013
 *      Author: sadra
 */

#ifndef EVOLUTIONARYMODABER_H_
#define EVOLUTIONARYMODABER_H_

#include "Modaber.h"
#include "CVC4Problem.h"
#include "Translator.h"
#include "SketchyPlan.h"
#include <vector>


using namespace std;

class EvolutionaryModaber : public Modaber {
private:

	CVC4Problem * smtProblem;
	Translator *myTranslator;

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve();

public:
	EvolutionaryModaber(char *domainFilePath, char *problemFilePath);

	double calculateFitness (SketchyPlan *sketchyPlan);

	virtual ~EvolutionaryModaber();
};

#endif /* EVOLUTIONARYMODABER_H_ */
