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

	Translator *myTranslator;

	CVC4Problem *smtProblem;



	int populationSize;
	int successfulChromosome;

	int nParents;

	double mutationRatio;


	int lengthOfChromosomes;


	double improvementThreshold;
	double lastConsideredFitness;
	int numberOfNonImprovingGeneration;
	int maximumNumberOfNonImprovingGeneration;



	void calculateFitness (SketchyPlan *sketchyPlan);

	void calculateFitness (vector <SketchyPlan> &population);

	void increasingLength(vector <SketchyPlan> &population);

	void crossover (vector <SketchyPlan> &population, vector <SketchyPlan> &children);

	void mutation (vector <SketchyPlan> &population, vector <SketchyPlan> &children, double mutationRatio);

	void selectNextGeneration(vector <SketchyPlan> &populations, vector <SketchyPlan> &nextGeneration);

	void selectParents (vector <SketchyPlan> &population, vector <const SketchyPlan*> &parents, int nParents);

	void testSketchyPlan ();

protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);

	virtual bool tryToSolve();

	void extractSolution(ostream &oss, CVC4Problem *smtProblem);


public:

	EvolutionaryModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph);

	virtual ~EvolutionaryModaber();
};

#endif /* EVOLUTIONARYMODABER_H_ */
