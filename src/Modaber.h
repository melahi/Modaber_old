/* In the name of God  */

#ifndef MODABER_H_
#define MODABER_H_

#include "VALfiles/TIM.h"
#include "ProblemPrinter.h"
#include <iostream>

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

class Modaber {
private:

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
	    ProblemPrinter myPrinter;
	    myPrinter.printProblem();
	}

public:
	Modaber(char *domainFilePath, char *problemFilePath){
		instantiation(domainFilePath, problemFilePath);
//		Result result = Result::UNSAT;
//		int nSignificantTimePoint = 1;
//		while (result != Result::SAT){
//
//			nSignificantTimePoint++;
//		}
	}
	virtual ~Modaber(){};
};

#endif /* MODABER_H_ */
