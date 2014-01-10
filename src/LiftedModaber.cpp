

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

	planGraph = new PlanningGraph();

	myProblem.liftedInitializing();

	liftedSMTProblem = new LiftedCVC4Problem(myProblem.nVariableIDs, myProblem.nPropositionIDs, myProblem.nPartialActions, myProblem.nUnification);
	myLiftedTranslator = new LiftedTranslator(liftedSMTProblem);

	nSignificantTimePoints = 1;

}

bool LiftedModaber::tryToSolve(){
	double bound = infinite;
	if (current_analysis->the_problem->metric->opt == E_MAXIMIZE){
		bound = -infinite;
	}
	return tryToSolve(bound);
}

bool LiftedModaber::tryToSolve(double bound){

	bool foundSolution = false;

	while (!foundSolution){
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		cout << "Extending my graph" << endl;
		planGraph->constructingGraph(nSignificantTimePoints);
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


double LiftedModaber::findPlanValue (){
	return myLiftedTranslator->getMetricValue();
}


LiftedModaber::LiftedModaber(char *domainFilePath, char *problemFilePath, char *solutionFilePath) {
	initialization(domainFilePath, problemFilePath);
	char outputFile [1000];
	double bound = infinite;
	bool hasMetricFunction = (current_analysis->the_problem->metric != NULL);
	if (hasMetricFunction && current_analysis->the_problem->metric->opt == E_MAXIMIZE){
		bound = -infinite;
	}
	int lastIndex = 1;
	while (true){
		tryToSolve(bound);
		cout << "The plan is: " << endl;
		myLiftedTranslator->extractSolution(cout);
		sprintf(outputFile, "%s.%d", solutionFilePath, lastIndex++);
		ofstream fout (outputFile);
		myLiftedTranslator->extractSolution(fout);
		if (!hasMetricFunction){
			break;
		}
		double temp = findPlanValue();
		cout << "Metric value is: " << temp << endl;
		if ((current_analysis->the_problem->metric->opt == E_MINIMIZE && temp >= bound) || (current_analysis->the_problem->metric->opt == E_MAXIMIZE && temp <= bound)){
			cout << temp << ' ' << bound << current_analysis->the_problem->metric->opt << endl;
			CANT_HANDLE("INCREMENTAL IMPORVEMING, CAN'T IMPROVE THE METRIC FUNCTION!!!");
			break;
		}else{
			bound = temp;
		}
	}
}

LiftedModaber::~LiftedModaber() {
	delete (myLiftedTranslator);
	delete (liftedSMTProblem);
	delete (planGraph);
}

