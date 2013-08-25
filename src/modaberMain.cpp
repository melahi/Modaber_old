//In the name of God


#include "SimpleModaber.h"
#include "EvolutionaryModaber.h"
#include <iostream>
#include <string>

using namespace std;

enum algorithm {SimpleAlgorithm, EvolutionaryAlgorithm};




int main (int argc, char * argv[]){
	bool usingPlanningGraph = true;
	int domainFileIndex = -1;
	int problemFileIndex = -1;
	algorithm alg = SimpleAlgorithm;


	//Parsing arguments
	for (int i = 1; i < argc; ++i){
		if (argv[i][0] == '-' && argv[i][1] == '-'){
			//This argument is a parameter
			if ( strcmp(argv[i], "--algorithm") == 0){
				if ( strcmp (argv[++i], "Evolutionary") == 0){
					alg = EvolutionaryAlgorithm;
				}else if ( strcmp (argv[i], "Simple") == 0){
					alg = SimpleAlgorithm;
				}else {
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


	//Running planner
	if (alg == SimpleAlgorithm){
		SimpleModaber simpleModaber (argv[domainFileIndex], argv[problemFileIndex], usingPlanningGraph);
	}else{
		EvolutionaryModaber evolutionaryModaber(argv[domainFileIndex], argv[problemFileIndex], usingPlanningGraph);
	}

	return 0;
}
