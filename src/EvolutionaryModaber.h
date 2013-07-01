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
	int populationSize;
	double improvementThreshold;
	double lastConsideredFitness;
	int numberOfNonImprovingGeneration;
	int lengthOfChromosomes;
	int maximumNumberOfNonImprovingGeneration;


	void calculateFitness (SketchyPlan *sketchyPlan);

	void calculateFitness (vector <SketchyPlan> &population);

	void increasingLength(vector <SketchyPlan> &population);

	vector <SketchyPlan> crossover (vector <SketchyPlan> &population);

	vector <SketchyPlan> mutation (vector <SketchyPlan> &population);

	vector <SketchyPlan> selectNextGeneration(vector <SketchyPlan> &populations);

	int nextParent (int currentParent, vector <SketchyPlan> &population);

	void testSketchyPlan ();

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve();

public:

	EvolutionaryModaber(char *domainFilePath, char *problemFilePath);

	virtual ~EvolutionaryModaber();
};

#endif /* EVOLUTIONARYMODABER_H_ */
