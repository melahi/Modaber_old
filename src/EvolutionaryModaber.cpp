/*
 * EvolutionaryModaber.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: sadra
 */

#include "EvolutionaryModaber.h"
#include "NumericalPlanningGraph.h"


#include <vector>
#include <cvc4/cvc4.h>
#include <algorithm>
#include <limits>
#include "Utilities.h"

using namespace std;

using namespace CVC4;



void EvolutionaryModaber::initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph){
	Modaber::initialization(domainFilePath, problemFilePath, usingPlanningGraph, true);

	//Genetic Algorithm parameters
	lengthOfChromosomes = 1;
	if (usingPlanningGraph){
		nPG->constructingGraph(lengthOfChromosomes);
	}
	myTranslator->prepare(lengthOfChromosomes);

	maximumNumberOfNonImprovingGeneration = 20;
	improvementThreshold = 5;
	populationSize = 200;
}

void EvolutionaryModaber::calculateFitness(SketchyPlan *sketchyPlan){
	sketchyPlan->fitness = myTranslator->solve(sketchyPlan);
}

void EvolutionaryModaber::calculateFitness(vector <SketchyPlan> &population){
	int populationSize = population.size();
	for (int i = 0; i < populationSize; i++){
		calculateFitness(&population[i]);
	}
}

void EvolutionaryModaber::increasingLength(vector <SketchyPlan> &population){
	/*
	 * Please note that the input population should be sorted, because
	 * we want to extract the best individuals and eliminate the others
	 * and we do that by keeping just the beginning part of population
	 * array (keep the indices of "0" to the "(selectRatioFromLastGeneration * population.size())" of
	 * the array)
	 */


	double selectRatioFromLastGeneration = 0.9;
	int selectedPopulation = (selectRatioFromLastGeneration * population.size()) ;

	lengthOfChromosomes++;
	if (usingPlanningGraph){
		nPG->constructingGraph(lengthOfChromosomes);
	}
	myTranslator->prepare(lengthOfChromosomes);

	for (int i = 0; i < selectedPopulation; i++){
		population[i].increaseOneLayer();
	}

	for (int i = selectedPopulation; i < populationSize; i++){
		population[i] = SketchyPlan(lengthOfChromosomes);
	}

	calculateFitness(population);
}

int EvolutionaryModaber::nextParent (int currentParent, vector <SketchyPlan> &population){
	double maximumParentSelectionProbability = 2 ; //This is the probability of choosing best chromosome as a parent
	int last = population.size();

	for (++currentParent; currentParent < last; ++currentParent){
		if (drand48() <= (maximumParentSelectionProbability * population[currentParent].fitness / population[0].fitness)){
			return currentParent;
		}
	}
	return currentParent;
}

vector <SketchyPlan> EvolutionaryModaber::crossover (vector  <SketchyPlan> &population){
	vector <SketchyPlan> crossoverChilds;
	int lastIndex = population.size();
	int motherIndex, fatherIndex;
	motherIndex = fatherIndex = -1;

	//Find first father and mother
	fatherIndex = nextParent(fatherIndex, population);
	motherIndex = nextParent(motherIndex, population);
	if (motherIndex == fatherIndex){
		motherIndex = nextParent(motherIndex, population);
	}

	while (motherIndex < lastIndex && fatherIndex < lastIndex){
		crossoverChilds.push_back(population[fatherIndex].crossover(&population[motherIndex]));

		//Find next father and mother
		fatherIndex = nextParent(fatherIndex, population);
		motherIndex = nextParent(motherIndex, population);
		if (motherIndex == fatherIndex){
			motherIndex = nextParent(motherIndex, population);
		}
	}
	return crossoverChilds;
}

vector <SketchyPlan> EvolutionaryModaber::mutation (vector <SketchyPlan> &population){
	double mutationRatio = 0.2;
	vector <SketchyPlan> mutationChilds;
	int last = population.size();

	for (int i = 0; i < last; i++){
		if (drand48() <= mutationRatio){
			mutationChilds.push_back(population[i].mutate());
		}
	}

	return mutationChilds;
}

vector <SketchyPlan> EvolutionaryModaber::selectNextGeneration (vector <SketchyPlan> &population){
	int successfulChromosome = populationSize * 0.4;
	int selectedChromosome = 0;

	vector <SketchyPlan> nextGeneration;

	sort (population.begin(), population.end());

	//First select successful chromosome
	while (selectedChromosome < successfulChromosome){
		nextGeneration.push_back (population[selectedChromosome]);
		++selectedChromosome;
	}

	//At next select lucky chromosome
	vector <int> luckyChromosomeIndex = selectRandomly(selectedChromosome, population.size(), populationSize - selectedChromosome);
	for (unsigned int i = 0; i < luckyChromosomeIndex.size(); i++){
		nextGeneration.push_back(population[luckyChromosomeIndex[i]]);
	}

	return nextGeneration;
}


bool EvolutionaryModaber::tryToSolve(){
	vector <SketchyPlan> population;
	bool foundSolution = false;
	int generationNumber = 1;
	lastConsideredFitness = 0;
	numberOfNonImprovingGeneration = 0;

	//Create random population for first generation
	for (int i = 0; i < populationSize; i++){
		population.push_back(SketchyPlan(lengthOfChromosomes));
	}

	calculateFitness(population);

	//Start Genetic Algorithm
	while (!foundSolution){
		cout << "Generation number: " << generationNumber << ", with the length of: " << lengthOfChromosomes << endl;

		sort(population.begin(), population.end());
		cout << "Best fitness: " << population[0].fitness << endl;
		if (population[0].fitness == numeric_limits <double>::max()){
			population[0].print();
			foundSolution = true;
			return true;
		}
		if (population[0].fitness - lastConsideredFitness >= improvementThreshold){
			lastConsideredFitness = population[0].fitness;
			numberOfNonImprovingGeneration = 0;
		}else{
			numberOfNonImprovingGeneration++;
		}

		if (numberOfNonImprovingGeneration >= maximumNumberOfNonImprovingGeneration){
			increasingLength(population);
			numberOfNonImprovingGeneration = 0;
		}

		vector <SketchyPlan> crossoverChilds = crossover (population);
		calculateFitness(crossoverChilds);

		vector <SketchyPlan> mutationChilds = mutation (population);
		calculateFitness(mutationChilds);

		population.insert(population.end(), crossoverChilds.begin(), crossoverChilds.end());
		population.insert(population.end(), mutationChilds.begin(), mutationChilds.end());

		population = selectNextGeneration(population);
		generationNumber++;
	}

	return false;
}

void EvolutionaryModaber::testSketchyPlan (){
	vector <SketchyPlan> parents;
	parents.push_back(SketchyPlan (20));
	parents.push_back(SketchyPlan (20));
	parents[0].print();
	cout << "*************************" << endl;
	parents[1].print();
	cout << "*************************" << endl;
	parents[0].crossover(&parents[1]).print();
	cout << "*************************" << endl;
	parents[0].mutate().print();
	cout << "*************************" << endl;
}

EvolutionaryModaber::EvolutionaryModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph) {
	initialization(domainFilePath, problemFilePath, usingPlanningGraph);


	bool foundSolution;
	foundSolution = tryToSolve();
	if (foundSolution){
		cout << "The plan is: " << endl;
		extractSolution(cout, smtProblem);
		cout << endl << endl;
		cout << "Modaber finished his task!!! ;)" << endl;
		cout << "*******************************" << endl;
		ofstream fout ("solution");
		extractSolution(fout, smtProblem);
	}
}

EvolutionaryModaber::~EvolutionaryModaber() {
	delete (myTranslator);
	delete (smtProblem);
}

