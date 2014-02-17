//In the name of God


#include "LiftedModaber.h"
#include "SatRangedModaber.h"
#include "EStepModaber.h"
#include <iostream>
#include <string>

using namespace std;
using namespace mdbr;

enum algorithm { SimpleSMT, PartialActionSMT, RangeSAT, EStepRange };


int main (int argc, char * argv[]){
 	int domainFileIndex = -1;
	int problemFileIndex = -1;
	int solutionFileIndex = -1;

	algorithm alg = EStepRange;

	//Parsing arguments
	for (int i = 1; i < argc; ++i){

		if (argv[i][0] == '-' && argv[i][1] == '-'){
			//This argument is a parameter
			if ( strcmp(argv[i], "--algorithm") == 0){
				++i;
				if ( strcmp (argv[i], "Simple") == 0){
					alg = SimpleSMT;
				}else if (strcmp (argv[i], "PartialActionSMT") == 0){
					alg = PartialActionSMT;
				}else if (strcmp (argv[i], "RangeSAT") == 0){
					alg = RangeSAT;
				}else if (strcmp (argv[i], "EStepRange") == 0){
					alg = EStepRange;
				}else {
					cerr << "undefined algorithm: " << argv[i] << endl;
					exit (0);
				}
			}else{
			cerr << "undefined option: " << argv[i] << endl;
			}
		}else if(domainFileIndex == -1){
			domainFileIndex = i;
		}else if (problemFileIndex == -1){
			problemFileIndex = i;
		}else if (solutionFileIndex == -1){
			solutionFileIndex = i;
		}else {
			cerr << "Can't handle argument: " << argv[i] << endl;
			exit (1);
		}
	}

	if (problemFileIndex == -1 || domainFileIndex == -1 || solutionFileIndex == -1){
		cerr << "Problem and/or Domain and/or Solution file/files is/are not specified!!" << endl;
		exit (1);
	}

	//Print input information
	cout << "Domain: " << argv[domainFileIndex] << endl;
	cout << "Problem: " << argv[problemFileIndex] << endl;
	cout << "Solution: " << argv[solutionFileIndex] << endl;


	if (alg == PartialActionSMT){
		LiftedModaber liftedModaber (argv[domainFileIndex], argv[problemFileIndex], argv[solutionFileIndex]);
	}else if (alg == RangeSAT){
		SatRangedModaber satRangeModaber (argv[domainFileIndex], argv[problemFileIndex], argv[solutionFileIndex]);
	}else if (alg == EStepRange){
		EStepModaber eStepModaber (argv[domainFileIndex], argv[problemFileIndex], argv[solutionFileIndex]);
	}

	return 0;
}
