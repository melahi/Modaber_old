/*
 * EvolutionaryModaber.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: sadra
 */

#include "EvolutionaryModaber.h"


void EvolutionaryModaber::initialization(char *domainFilePath, char *problemFilePath){
	Modaber::initialization(domainFilePath, problemFilePath);
	smtProblems.push_back(new CVC4Problem(instantiatedOp::howManyNonStaticPNEs(), instantiatedOp::howManyNonStaticLiterals(), instantiatedOp::howMany()));
	myTranslator = new Translator(smtProblems[0], myAnalyzer, numericRPG);
	myTranslator->addInitialState();
	smtProblems.push_back(new CVC4Problem(*smtProblems[0]));
	myTranslator->addGoals(0);
}

//The following function set suitable smtProblem for myTranslator according to the number of significant time point
void EvolutionaryModaber::prepareTranslatorFor (unsigned int nSignificantTimePoint){
	unsigned int smtProblemsSize = smtProblems.size();

	//The last smtProblem is not completed so even if nSignificantPoint corresponds to
	//the last element of smtProblems, we need to expand it first.
	while (smtProblemsSize <= nSignificantTimePoint){
		myTranslator->setSMTProblem(smtProblems[smtProblemsSize - 1]);
		smtProblems[smtProblemsSize - 1]->guaranteeSize(smtProblemsSize);
		myTranslator->addActions(smtProblemsSize - 2);
		myTranslator->addActionMutex(smtProblemsSize - 2);
		myTranslator->addExplanatoryAxioms(smtProblemsSize - 1);
		smtProblems.push_back(new CVC4Problem(*smtProblems[smtProblemsSize - 1]));
		myTranslator->addGoals(smtProblemsSize - 1);
		smtProblemsSize++;
	}
	myTranslator->setSMTProblem(smtProblems[nSignificantTimePoint - 1]);

}

double EvolutionaryModaber::calculateFitness(SketchyPlan *sketchyPlan){
	unsigned int length = sketchyPlan->milestones.size();
	prepareTranslatorFor(length);
	myTranslator->getSMTProblem()->push();
	sketchyPlan->print();
	myTranslator->addSkechyPlan(sketchyPlan);
	bool foundSolution = myTranslator->getSMTProblem()->solve();
	myTranslator->getSMTProblem()->pop();
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
		extractSolution(cout, myTranslator->getSMTProblem());
		cout << endl << endl;
		cout << "Modaber finished his task!!! ;)" << endl;
		cout << "*******************************" << endl;
		ofstream fout ("solution");
		extractSolution(fout, myTranslator->getSMTProblem());
	}
}

EvolutionaryModaber::~EvolutionaryModaber() {
	delete (myTranslator);
}

