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
#include "NumericRPG.h"
#include "SketchyPlan.h"

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

void SimpleModaber::initialization(char *domainFilePath, char *problemFilePath){
	Modaber::initialization(domainFilePath, problemFilePath);
	smtProblem = new CVC4Problem(instantiatedOp::howManyNonStaticPNEs(), instantiatedOp::howManyNonStaticLiterals(), instantiatedOp::howMany());
	nSignificantTimePoint = 1;
}

bool SimpleModaber::tryToSolve(){

	bool foundSolution = false;
	Translator myTranslator (smtProblem, myAnalyzer, numericRPG);
	myTranslator.addInitialState();

	while (!foundSolution){
		smtProblem->push();
		myTranslator.addGoals(nSignificantTimePoint - 1);
		cout  << "nSignificantTimePoint: " << nSignificantTimePoint << endl;
		foundSolution = smtProblem->solve();
		if (!foundSolution){
			nSignificantTimePoint++;
			smtProblem->pop();
			smtProblem->guaranteeSize(nSignificantTimePoint);
			myTranslator.addActions(nSignificantTimePoint - 2);
			myTranslator.addActionMutex(nSignificantTimePoint - 2);
			myTranslator.addExplanatoryAxioms(nSignificantTimePoint - 1);
		}
	}
	return foundSolution;
}



SimpleModaber::SimpleModaber(char *domainFilePath, char *problemFilePath) {
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

SimpleModaber::~SimpleModaber() {
	delete (smtProblem);
}
