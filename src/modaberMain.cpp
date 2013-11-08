//In the name of God


#include "SimpleModaber.h"
#include "LiftedModaber.h"
#include <iostream>
#include <string>

using namespace std;
using namespace mdbr;

enum algorithm {SimpleAlgorithm, LiftedAlgorithm};


int main (int argc, char * argv[]){
	bool usingPlanningGraph = true;
	int domainFileIndex = -1;
	int problemFileIndex = -1;
	algorithm alg = LiftedAlgorithm;


	//Parsing arguments
	for (int i = 1; i < argc; ++i){
		if (argv[i][0] == '-' && argv[i][1] == '-'){
			//This argument is a parameter
			if ( strcmp(argv[i], "--algorithm") == 0){
				++i;
				if ( strcmp (argv[i], "Simple") == 0){
					alg = SimpleAlgorithm;
				}else if (strcmp (argv[i], "Lifted") == 0){
					alg = LiftedAlgorithm;
				}
/*
				else if ( strcmp (argv[++i], "Evolutionary") == 0){
					alg = EvolutionaryAlgorithm;
				}
*/
				else  {
					cerr << "undefined algorithm: " << argv[i] << endl;
					exit (0);
				}
			}else if ( strcmp (argv[i], "--NPG") == 0) {
				if ( strcmp (argv[++i], "0") == 0){
					usingPlanningGraph = false;
				}else if ( strcmp (argv[i], "1") == 0){
					usingPlanningGraph = true;
				}else{
					cerr << "undefined parameter for NPG: " << argv[i] << endl;
					exit (0);
				}
			}else {
				cerr << "undefined parameter: " << argv[i] << endl;
			}
		}else if (domainFileIndex == -1){
			domainFileIndex = i;
		}else{
			problemFileIndex = i;
		}
	}

	if (problemFileIndex == -1 || domainFileIndex == -1){
		cerr << "Problem and/or Domain is not specified!!" << endl;
		exit (1);
	}

	//Print input information
	cout << "Domain: " << argv[domainFileIndex] << endl;
	cout << "Problem: " << argv[problemFileIndex] << endl;
	cout << "Using Planning Graph: " << (usingPlanningGraph ? "Yes" : "No") << endl;
	cout << "Algorithm: ";
	if (alg == SimpleAlgorithm){
		cout << "Simple Algorithm" << endl;
	}else if (alg == LiftedAlgorithm) {
		cout << "Lifted Algorithm" << endl;
	}


	//Running planner
	if (alg == SimpleAlgorithm){
		SimpleModaber simpleModaber (argv[domainFileIndex], argv[problemFileIndex], usingPlanningGraph);
	}else {
		LiftedModaber liftedModaber (argv[domainFileIndex], argv[problemFileIndex], usingPlanningGraph);
	}

/*
	else{
		EvolutionaryModaber evolutionaryModaber(argv[domainFileIndex], argv[problemFileIndex], usingPlanningGraph);
	}
*/


	return 0;
}
