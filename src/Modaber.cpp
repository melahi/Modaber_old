/* In the name of God  */

#include "Modaber.h"
#include "VALfiles/TIM.h"
#include "ProblemPrinter.h"
#include "Translator.h"
#include <iostream>
#include <fstream>
#include "CVC4Problem.h"
#include "PlanningGraph.h"
#include "MyProblem.h"

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;


using namespace mdbr;

void Modaber::instantiation(char *domainFile, char *problemFile){
	char *argv[2];
	argv[0] = domainFile;
	argv[1] = problemFile;
	performTIMAnalysis(argv);
	SimpleEvaluator::setInitialState();

	for (operator_list::const_iterator os = current_analysis->the_domain->ops->begin();
			os != current_analysis->the_domain->ops->end(); ++os) {
		instantiatedOp::instantiate(*os, current_analysis->the_problem, *theTC);
	};

	cout << "Before filtering: " << instantiatedOp::howMany() << endl;
	{
		int fpass = 1;
		int numBefore;
		do {
			instantiatedOp::createAllLiterals(current_analysis->the_problem, theTC);
			numBefore = instantiatedOp::howMany();
			instantiatedOp::filterOps(theTC);
			++fpass;
		} while (instantiatedOp::howMany() < numBefore);
	}

	instantiatedOp::assignStateIDsToNonStaticLiteralsAndPNEs();

	cout << "After filtering: " << instantiatedOp::howMany() << endl;

//	ProblemPrinter myPrinter;
//	myPrinter.printProblem();
}

void Modaber::initialization(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph, bool usingSASPlus){
	instantiation(domainFilePath, problemFilePath);
	myProblem.initializing(usingSASPlus);
	nPG = new PlanningGraph();
	this->usingPlanningGraph = usingPlanningGraph;
	if (!usingPlanningGraph){
		nPG->ignoreGraph();
	}
}


Modaber::~Modaber(){
	delete (nPG);
}

