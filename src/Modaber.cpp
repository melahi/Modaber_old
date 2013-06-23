/* In the name of God  */

#include "Modaber.h"
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
	//@TODO: I should do something for static literals and PNEs (In translation make a difference between them and the others)

	cout << "After filtering: " << instantiatedOp::howMany() << endl;

	//ProblemPrinter myPrinter;
	//myPrinter.printProblem();
}

void Modaber::initialization(char *domainFilePath, char *problemFilePath){
	instantiation(domainFilePath, problemFilePath);
	CVC4Problem::updateInitialValues();
	myAnalyzer = new MyAnalyzer();
	numericRPG = new NumericRPG();
}

void Modaber::extractSolution(ostream &oss, CVC4Problem *smtProblem){
	//Generating output
	unsigned int maximumSignificantTimePoint = smtProblem->getMaximumSignificantTimePoint();
	int nAction = instantiatedOp::howMany();
	for (unsigned int i = 0; i < maximumSignificantTimePoint - 1; i++){
		for (int j = 0; j < nAction; j++){
			if (smtProblem->isActionUsed(j, i)){
				instantiatedOp *action = instantiatedOp::getInstOp(j);
				action->write(oss);
				oss << endl;
			}
		}
	}
}





Modaber::Modaber(){}


Modaber::~Modaber(){
	delete (myAnalyzer);
	delete (numericRPG);
}

