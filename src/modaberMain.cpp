//In the name of God


#include "LiftedModaber.h"
#include <iostream>
#include <string>

using namespace std;
using namespace mdbr;


int main (int argc, char * argv[]){
	int domainFileIndex = -1;
	int problemFileIndex = -1;


	//Parsing arguments
	for (int i = 1; i < argc; ++i){
		if (domainFileIndex == -1){
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

	LiftedModaber liftedModaber (argv[domainFileIndex], argv[problemFileIndex]);

	return 0;
}
