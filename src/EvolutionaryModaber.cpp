/*
 * EvolutionaryModaber.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: sadra
 */

#include "EvolutionaryModaber.h"


#include <vector>
#include <cvc4/cvc4.h>
using namespace std;

using namespace CVC4;



void EvolutionaryModaber::initialization(char *domainFilePath, char *problemFilePath){
	Modaber::initialization(domainFilePath, problemFilePath);
	smtProblem = new CVC4Problem(instantiatedOp::howManyNonStaticPNEs(), instantiatedOp::howManyNonStaticLiterals(), instantiatedOp::howMany());
	myTranslator = new Translator(smtProblem, myAnalyzer, numericRPG);
}

double EvolutionaryModaber::calculateFitness(SketchyPlan *sketchyPlan){
	unsigned int length = sketchyPlan->milestones.size();
	cout << "Now we are going to attack the problem" << endl;
	cout << "Start pushing" << endl;
//	myTranslator->getSMTProblem()->push();
	cout << "End pushing" << endl;

	sketchyPlan->print();

	bool foundSolution = myTranslator->solve(length, sketchyPlan);
//	myTranslator->getSMTProblem()->pop();
	cout << "fitness value for the sketchy plan: " << foundSolution << endl;
	return foundSolution;
}

bool EvolutionaryModaber::tryToSolve(){
	vector <SketchyPlan> population;
	for (int i = 0; i < 1000; i++){
		population.push_back(SketchyPlan(numericRPG));
		cout << "calculate fitness for sketchy plan: " << i << endl;
		if (fabs (1 - calculateFitness(&population[i])) < EPSILON){
			return true;
		}
	}
	return false;
}


EvolutionaryModaber::EvolutionaryModaber(char *domainFilePath, char *problemFilePath) {
	initialization(domainFilePath, problemFilePath);
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
}

