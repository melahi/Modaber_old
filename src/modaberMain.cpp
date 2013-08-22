//In the name of God


#include "SimpleModaber.h"
#include "EvolutionaryModaber.h"

using namespace std;




int main (int argc, char * argv[]){
	bool usingPlanningGraph = true;

	if (argc > 3 && argv[3][0] == '0'){
		cout << "Numerical Planning Graph has been disabled!" << endl;
		usingPlanningGraph = false;
	}else{
		cout << argc << ' ' << argv[argc - 1] << endl;
	}

//	SimpleModaber simpleModaber (argv[1], argv[2], usingPlanningGraph);

	EvolutionaryModaber evolutionaryModaber(argv[1], argv[2], usingPlanningGraph);

	return 0;
}
