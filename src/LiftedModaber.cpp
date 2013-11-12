

#include "LiftedModaber.h"


#include "VALfiles/TIM.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

using namespace mdbr;

void LiftedModaber::initialization (char *domainFilePath, char *problemFilePath, bool usingPlanningGraph){
	Modaber::initialization(domainFilePath, problemFilePath, usingPlanningGraph, false);



	if (usingPlanningGraph){
		nPG->constructingGraph();
	}
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
		myLiftedTranslator->prepare(nSignificantTimePoints);
		foundSolution = myLiftedTranslator->solve();
		cout << "end solving" << endl;
		if (!foundSolution){
				nSignificantTimePoints += 5;
		}
	}
	return foundSolution;
}



LiftedModaber::LiftedModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph) {
	initialization(domainFilePath, problemFilePath, usingPlanningGraph);
	bool foundSolution;
	foundSolution = tryToSolve();
	if (foundSolution){
		cout << "The plan is: " << endl;
		myLiftedTranslator->extractSolution(cout);
		cout << endl << endl;
		cout << "Modaber finished his task!!! ;)" << endl;
		cout << "*******************************" << endl;
		ofstream fout ("solution");
		myLiftedTranslator->extractSolution(fout);
	}
}

LiftedModaber::~LiftedModaber() {
	delete (liftedSMTProblem);
	delete (myLiftedTranslator);
}

