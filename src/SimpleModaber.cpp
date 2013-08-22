/*
 * SimpleModaber.cpp
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#include "SimpleModaber.h"


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

void SimpleModaber::initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph){
	Modaber::initialization(domainFilePath, problemFilePath, usingPlanningGraph, false);
	nSignificantTimePoint = 1;
	if (usingPlanningGraph){
		nPG->constructingGraph(nSignificantTimePoint);
	}
}

bool SimpleModaber::tryToSolve(){

	bool foundSolution = false;
	while (!foundSolution){
		myTranslator->prepare(nSignificantTimePoint);
		cout  << "nSignificantTimePoint: " << nSignificantTimePoint << endl;
		cout << "solving ..." << endl;
		foundSolution = myTranslator->solve();
		cout << "end solving" << endl;
		if (!foundSolution){
			nSignificantTimePoint++;
			if (usingPlanningGraph){
				cout << "constructing NGP" << endl;
				nPG->constructingGraph(nSignificantTimePoint);
				cout << "end constructing" << endl;
			}
		}
	}
	return foundSolution;
}



SimpleModaber::SimpleModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph) {
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

SimpleModaber::~SimpleModaber() {
	delete (myTranslator);
	delete (smtProblem);
}

