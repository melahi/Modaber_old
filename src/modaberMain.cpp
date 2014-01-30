//In the name of God


#include "LiftedModaber.h"
#include <iostream>
#include <string>

using namespace std;
using namespace mdbr;

enum algorithm { SimpleSMT, PartialActionSMT, RangeSAT }


int main (int argc, char * argv[]){
 	int domainFileIndex = -1;
	int problemFileIndex = -1;
	int solutionFileIndex = -1;


	//Parsing arguments
	for (int i = 1; i < argc; ++i){
		if (domainFileIndex == -1){
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

	LiftedModaber liftedModaber (argv[domainFileIndex], argv[problemFileIndex], argv[solutionFileIndex]);

	return 0;
}
