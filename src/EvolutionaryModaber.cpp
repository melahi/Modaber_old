/*
 * EvolutionaryModaber.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: sadra
 */

#include "EvolutionaryModaber.h"
#include "PlanningGraph.h"


#include <vector>
#include <cvc4/cvc4.h>
#include <algorithm>
#include <limits>
#include "Utilities.h"

using namespace std;

using namespace CVC4;



void EvolutionaryModaber::initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph){
	Modaber::initialization(domainFilePath, problemFilePath, usingPlanningGraph, true);

	smtProblem = new CVC4Problem(instantiatedOp::howManyNonStaticPNEs(), instantiatedOp::howManyNonStaticLiterals(), instantiatedOp::howMany());
	myTranslator = new Translator(smtProblem);


	lengthOfChromosomes = 1;
	if (usingPlanningGraph){
		nPG->constructingGraph(lengthOfChromosomes);
	}
	myTranslator->prepare(lengthOfChromosomes);
	myProblem.environment.prepare(lengthOfChromosomes);


	//Genetic Algorithm parameters
	maximumNumberOfNonImprovingGeneration = 100;
	improvementThreshold = 1;
	populationSize = 200;
	successfulChromosome = populationSize * 0.2;
	mutationRatio = 0.2;



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
	myProblem.environment.prepare(lengthOfChromosomes);

	for (int i = 0; i < selectedPopulation; i++){
		population[i].increaseOneLayer();
	}

	for (int i = selectedPopulation; i < populationSize; i++){
		population[i] = SketchyPlan(lengthOfChromosomes);
	}

	calculateFitness(population);
}

void EvolutionaryModaber::selectParents (vector <SketchyPlan> &population, vector <const SketchyPlan*> &parents, int nParents){
	//This selection is based on roulette wheel method

	parents.resize(nParents, NULL);
	int populationSize = population.size();

	//Find probability distribution for selecting each individual as a parent
	vector <double> probabilityDistribution (populationSize, 0);
	double sum = 0;
	for (int i = 0; i < populationSize; i++) {
		probabilityDistribution[i] = population[i].fitness;
		sum += probabilityDistribution[i];
	}
	normolizing(probabilityDistribution, sum);


	//Selecting Parents
	for (int i = 0; i < nParents; i++){
		int selected = selectRandomly(probabilityDistribution);
		parents[i] = &(population[selected]);
	}
}

void EvolutionaryModaber::crossover (vector  <SketchyPlan> &population, vector <SketchyPlan> &children){
	vector <const SketchyPlan *> parents;
	selectParents(population, parents, nParents);

	children.resize(nParents / 2);
	for (int i = 0; i + 1 < nParents; i += 2){
		children[i / 2] = parents[i]->crossover(parents[i+1]);
	}
}

void EvolutionaryModaber::mutation (vector <SketchyPlan> &population, vector <SketchyPlan> &children, double mutationRatio){

	int last = population.size();

	for (int i = 0; i < last; i++){
		if (drand48() <= mutationRatio){
			children.push_back(population[i].mutate());
		}
	}
}

void EvolutionaryModaber::selectNextGeneration (vector <SketchyPlan> &population, vector <SketchyPlan> &nextGeneration){
	int selectedChromosome = 0;

	nextGeneration.resize(populationSize);

	sort (population.begin(), population.end());

	//First select successful chromosome
	while (selectedChromosome < successfulChromosome){
		nextGeneration[selectedChromosome]  = population[selectedChromosome];

		++selectedChromosome;
	}

	//At next select lucky chromosome
	vector <int> luckyChromosomeIndex = selectRandomly(selectedChromosome, population.size(), populationSize - selectedChromosome);
	for (unsigned int i = 0; i < luckyChromosomeIndex.size(); ++i, ++selectedChromosome){
		nextGeneration[selectedChromosome] = population[luckyChromosomeIndex[i]];
	}

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
		if (population[0].fitness == numeric_limits <int>::max()){
			population[0].write(cout);
			foundSolution = true;
			return true;
		}
		if (population[0].fitness - lastConsideredFitness >= improvementThreshold){
			lastConsideredFitness = population[0].fitness;
			numberOfNonImprovingGeneration = 0;
			mutationRatio = 0.2;
		}else{
			mutationRatio = 1;
			numberOfNonImprovingGeneration++;
		}

		if (numberOfNonImprovingGeneration >= maximumNumberOfNonImprovingGeneration){
			increasingLength(population);
			numberOfNonImprovingGeneration = 0;
		}

		vector <SketchyPlan> crossoverChildren;
		crossover (population, crossoverChildren);
		calculateFitness(crossoverChildren);

		vector <SketchyPlan> mutationChildren;
		mutation (population, mutationChildren, mutationRatio);
		calculateFitness(mutationChildren);

		population.insert(population.end(), crossoverChildren.begin(), crossoverChildren.end());
		population.insert(population.end(), mutationChildren.begin(), mutationChildren.end());


		vector <SketchyPlan> nextGeneration;
		selectNextGeneration(population, nextGeneration);
		population = nextGeneration;
		generationNumber++;
	}

	return false;
}



void EvolutionaryModaber::extractSolution(ostream &oss, CVC4Problem *smtProblem){
	//Generating output
	unsigned int maximumSignificantTimePoint = smtProblem->getMaximumSignificantTimePoint();
	int nAction = instantiatedOp::howMany();
	for (unsigned int i = 0; i < maximumSignificantTimePoint - 1; i++){
		for (int j = 0; j < nAction; j++){
			if (isVisited(myProblem.actions[j].firstVisitedLayer, (int) i) && smtProblem->isActionUsed(j, i)){
				instantiatedOp *action = instantiatedOp::getInstOp(j);
				action->write(oss);
				oss << endl;
			}
		}
	}
}

void EvolutionaryModaber::testSketchyPlan (){
	vector <SketchyPlan> parents;
	parents.push_back(SketchyPlan (8));
	parents.push_back(SketchyPlan (8));
	parents[0].write(cout);
	cout << "*************************" << endl;
	parents[1].write(cout);
	cout << "*************************" << endl;
	parents[0].crossover(&parents[1]).write(cout);
	cout << "*************************" << endl;
	parents[0].mutate().write(cout);
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

