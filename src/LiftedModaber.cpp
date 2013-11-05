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
	bool usingUndefinedVariables = true;
	bool updatingValues;
	int nUnsuccessfulSolution = 0;
	int unsuccessfulSolutionThreshold = 20;

	while (!foundSolution){
		updatingValues = false;
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		cout << "solving ..." << endl;
		translator.prepare(myProblem.operators.size(), myProblem.nPropositionVariables, myProblem.nUnification, myProblem.partialAction.size(), myProblem.assignments.size(), myProblem.comparisons.size(), myProblem.nValues);
		foundSolution = translator.solve(nSignificantTimePoints);
		cout << "end solving" << endl;
		if (!foundSolution){
			if (usingUndefinedVariables){
				nSignificantTimePoints += 5;
			}else{
				usingUndefinedVariables = true;
				simulator.addUndefinedValues();
				updatingValues = true;
			}
		}else{

			vector <pair <operator_ *, FastEnvironment> > solution;
			translator.getSolution(solution);
			int nValuesBeforeSimulation = simulator.countValues();
			foundSolution =	simulator.isValid(solution);
			int nValuesAfterSimulation = simulator.countValues();

			if (!foundSolution){
				translator.printSolution(cout);
				cout << endl << endl << "------------------" << endl << "A plan has been found but is not valid!!!" << endl << endl;
				nUnsuccessfulSolution++;
				if (nValuesAfterSimulation == nValuesBeforeSimulation || nUnsuccessfulSolution > unsuccessfulSolutionThreshold){
					usingUndefinedVariables = false;
					simulator.removeUndefinedValues();
				}else{
					usingUndefinedVariables = true;
					simulator.addUndefinedValues();
				}
				updatingValues = true;
			}
		}


		if (updatingValues){
			myProblem.assignIdToValues();
			int nVariables = myProblem.variables.size();
			for (int i = 0; i < nVariables; ++i){
				myProblem.variables[i].completeDomainRange();
				myProblem.variables[i].write(cout);
				cout << endl;
			}
			list <MyComparison>::iterator cmpIt, cmpItEnd;
			cmpIt = myProblem.comparisons.begin();
			cmpItEnd = myProblem.comparisons.end();

			for (; cmpIt != cmpItEnd; ++cmpIt){
//				cout << "=====" << cmpIt->possibleValues.size();
				cmpIt->findPossibleRanges();
//				cmpIt->write(cout);
//				cout << "=====" << cmpIt->possibleValues.size() << endl;

			}

			list <MyAssignment>::iterator asgnIt, asgnItEnd;
			asgnIt = myProblem.assignments.begin();
			asgnItEnd = myProblem.assignments.end();

			for (; asgnIt != asgnItEnd; ++asgnIt){
//				cout << "*****" << asgnIt->possibleValues.size();
				asgnIt->findPossibleRanges();
//				asgnIt->write(cout);
//				cout << "*****" << asgnIt->possibleValues.size() << endl;
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

