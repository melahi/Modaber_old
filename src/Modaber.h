/* In the name of God  */

#ifndef MODABER_H_
#define MODABER_H_

#include "VALfiles/TIM.h"
#include "ProblemPrinter.h"
#include "Translator.h"
#include <iostream>
#include <fstream>
#include "CVC4Problem.h"
#include "NumericRPG.h"

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

class Modaber {
private:
	int nSignificantTimePoint;

	CVC4Problem *smtProblem;
	MyAnalyzer *myAnalyzer;
	NumericRPG *numericRPG;

	void instantiation(char *domainFile, char *problemFile){
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

	void initialization(char *domainFilePath, char *problemFilePath){
		instantiation(domainFilePath, problemFilePath);
		CVC4Problem::updateInitialValues();
		smtProblem = new CVC4Problem(instantiatedOp::howManyNonStaticPNEs(), instantiatedOp::howManyNonStaticLiterals(), instantiatedOp::howMany());
		myAnalyzer = new MyAnalyzer();
		numericRPG = new NumericRPG();
		nSignificantTimePoint = 1;
	}

	bool tryToSolve(){

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
				smtProblem->increaseSizeForAnotherStep();
				myTranslator.addActions(nSignificantTimePoint - 2);
				myTranslator.addActionMutex(nSignificantTimePoint - 2);
				myTranslator.addExplanatoryAxioms(nSignificantTimePoint - 1);
			}
		}
		return foundSolution;
	}

	void extractSolution(ostream &oss){
		//Generating output
		int nAction = instantiatedOp::howMany();
		for (int i = 0; i < nSignificantTimePoint - 1; i++){
			for (int j = 0; j < nAction; j++){
				if (smtProblem->isActionUsed(j, i)){
					instantiatedOp *action = instantiatedOp::getInstOp(j);
					action->write(oss);
					oss << endl;
				}
			}
		}
	}



public:

	Modaber(char *domainFilePath, char *problemFilePath): myAnalyzer(){
		initialization(domainFilePath, problemFilePath);
		bool foundSolution;
		foundSolution = tryToSolve();
		if (foundSolution){
			cout << "The plan is: " << endl;
			extractSolution(cout);
			cout << endl << endl;
			cout << "Modaber finished his task!!! ;)" << endl;
			cout << "*******************************" << endl;
			ofstream fout ("solution");
			extractSolution(fout);
		}
	}


	virtual ~Modaber(){
		delete (smtProblem);
		delete (myAnalyzer);
		delete (numericRPG);
	}
};

#endif /* MODABER_H_ */
