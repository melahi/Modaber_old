

#include "LiftedModaber.h"


#include "VALfiles/TIM.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

using namespace mdbr;

void LiftedModaber::initialization (char *domainFilePath, char *problemFilePath){
	Modaber::initialization(domainFilePath, problemFilePath);



	myProblem.liftedInitializing();

	liftedSMTProblem = new LiftedCVC4Problem(myProblem.nVariableIDs, myProblem.nPropositionIDs, myProblem.nPartialActions, myProblem.nUnification);
	myLiftedTranslator = new LiftedTranslator(liftedSMTProblem);

	nSignificantTimePoints = 1;

}

bool LiftedModaber::tryToSolve(){

	bool foundSolution = false;

	while (!foundSolution){
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		cout << "solving ..." << endl;
		myLiftedTranslator->prepare(nSignificantTimePoints, infinite);
		foundSolution = myLiftedTranslator->solve();
		cout << "end solving" << endl;
		if (!foundSolution){
				nSignificantTimePoints += 5;
		}
	}
	return foundSolution;
}

bool LiftedModaber::tryToSolve(double bound){

	bool foundSolution = false;

	while (!foundSolution){
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		cout << "solving ..." << endl;
		myLiftedTranslator->prepare(nSignificantTimePoints, bound);
		foundSolution = myLiftedTranslator->solve();
		cout << "end solving" << endl;
		if (!foundSolution){
			nSignificantTimePoints += 5;
		}
	}
	return foundSolution;
}


double LiftedModaber::findPlanValue (const char *domainFile, const char *problemFile, const char *solutionFile){
	char myCommand [10000];
	sprintf(myCommand, "./validate %s %s %s | grep \"Final value: \"", domainFile, problemFile, solutionFile);
	FILE *file = popen(myCommand, "r");
	double planValue;
	if ( feof(file) || 	fscanf(file, "Final value: %lf", &planValue) != 1){
		return infinite;
	}
	cout << "Plan Value is: " << planValue << endl;
	return planValue;
}

LiftedModaber::LiftedModaber(char *domainFilePath, char *problemFilePath) {
	initialization(domainFilePath, problemFilePath);
	char solutionFile [1000] = "solution";
	double bound = infinite;
	while (true){
		tryToSolve(bound);
		cout << "The plan is: " << endl;
		myLiftedTranslator->extractSolution(cout);
		ofstream fout (solutionFile);
		myLiftedTranslator->extractSolution(fout);
		bound = findPlanValue(domainFilePath, problemFilePath, solutionFile);
	}
}

LiftedModaber::~LiftedModaber() {
	delete (myLiftedTranslator);
	delete (liftedSMTProblem);
}

