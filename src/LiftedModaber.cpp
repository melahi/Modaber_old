/*
 * SimpleModaber.cpp
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#include "LiftedModaber.h"


#include "VALfiles/TIM.h"
#include "ProblemPrinter.h"
#include "Translator.h"
#include <iostream>
#include <fstream>
#include "CVC4Problem.h"
#include "SketchyPlan.h"

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

	nSignificantTimePoints = 1;

}

bool LiftedModaber::tryToSolve(){

	bool foundSolution = false;
	while (!foundSolution){
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		cout << "solving ..." << endl;
		translator.prepare(myProblem.operators.size(), myProblem.nPropositionVariables, myProblem.nUnification, myProblem.partialAction.size(), myProblem.assignments.size(), myProblem.comparisons.size(), myProblem.nValues);
		foundSolution = translator.solve(nSignificantTimePoints);
		cout << "end solving" << endl;
		if (!foundSolution){
			nSignificantTimePoints += 5;
		}else{
			vector <pair <operator_ *, FastEnvironment> > solution;
			translator.getSolution(solution);
			foundSolution =	simulator.isValid(solution);

			if (!foundSolution){
				translator.printSolution(cout);
				cout << endl << endl << "------------------" << endl << "A plan has been found but is not valid!!!" << endl << endl;
				myProblem.assignIdToValues();

				list <MyComparison>::iterator cmpIt, cmpItEnd;
				cmpIt = myProblem.comparisons.begin();
				cmpItEnd = myProblem.comparisons.end();

				for (; cmpIt != cmpItEnd; ++cmpIt){
					cmpIt->findPossibleValues();
//					cmpIt->write(cout);

				}

				list <MyAssignment>::iterator asgnIt, asgnItEnd;
				asgnIt = myProblem.assignments.begin();
				asgnItEnd = myProblem.assignments.end();

				for (; asgnIt != asgnItEnd; ++asgnIt){
					asgnIt->findPossibleValues();
//					asgnIt->write(cout);
				}
			}
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
		translator.printSolution(cout);
		cout << endl << endl;
		cout << "Modaber finished his task!!! ;)" << endl;
		cout << "*******************************" << endl;
		ofstream fout ("solution");
		translator.printSolution(fout);
	}
}

LiftedModaber::~LiftedModaber() {
}

