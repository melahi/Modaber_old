/*
 * MyProblem.cpp
 *
 *  Created on: Aug 6, 2013
 *      Author: sadra
 */

#include "MyProblem.h"
#include "Utilities.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>



using namespace VAL;
using namespace Inst;
using namespace std;

namespace mdbr {

MyProblem myProblem;


void MyProblem::filterVariables(){
	int nVariables = variables.size();

	for (int i = 0; i < nVariables; i++){

		//For eliminating variables from Numerical Planning Graph
		//The following line should be eliminated as soon as possible
		variables[i].visitInPrecondition = false;


		if (variables[i].visitInPrecondition == true){
			continue;
		}
		list <MyAction *>::iterator it, itEnd;
		it = variables[i].userActions.begin();
		itEnd = variables[i].userActions.end();
		for (; it != itEnd; ++it){
			(*it)->variableNeeded.erase(&variables[i]);
		}
		it = variables[i].modifierActions.begin();
		itEnd = variables[i].modifierActions.end();
		for (; it != itEnd; ++it){
			(*it)->modifyingVariable.erase(&variables[i]);
		}
	}
}

void MyProblem::buildingDTG(){
	int nActions = actions.size();
	for (int i = 0; i < nActions; i++){
		vector <int> deleteStateValue (stateVariables.size(), -1);
		vector <int> addStateValue (stateVariables.size(), -1);
		set <MyProposition*>::iterator it1, itEnd1;
		list <MyProposition*>::iterator it2, itEnd2;
		itEnd1 = actions[i].deleteList.end();
		itEnd2 = actions[i].addList.end();
		for (it1 = actions[i].deleteList.begin(); it1 != itEnd1; ++it1){
			if ((*it1)->stateValue != NULL){
				deleteStateValue[ (*it1)->stateValue->theStateVariable->variableId ] = (*it1)->stateValue->valueId;
			}
		}
		for (it2 = actions[i].addList.begin(); it2 != itEnd2; ++it2){
			if ((*it2)->stateValue != NULL){
				addStateValue[ (*it2)->stateValue->theStateVariable->variableId ] = (*it2)->stateValue->valueId;
			}
		}
		int nStateVariables = stateVariables.size();
		for (int j = 0; j < nStateVariables; j++){
			if (deleteStateValue[j] != -1){
				if (addStateValue[j] != -1){
					stateVariables[j].domain[addStateValue[j]].providers[deleteStateValue[j]].push_back(&actions[i]);
				}else { /* if addStateValue[j] == -1 means the action change the corresponding state value to the "<none of those>" state value */
					stateVariables[j].domain[stateVariables[j].domain.size() - 1].providers[deleteStateValue[j]].push_back(&actions[i]);
				}
			}else if (addStateValue[j] != 1){
				stateVariables[j].domain[addStateValue[j]].providers[stateVariables[j].domain.size() - 1].push_back(&actions[i]);
			}
		}
	}
}


void MyProblem::readingSASPlusFile(){
	map <string, MyProposition*> propositionName;
	int propositionsSize = propositions.size();
	for (int i = 0; i < propositionsSize; i++){
		ostringstream sout;
		propositions[i].originalLiteral->myWrite(sout);
		propositionName [sout.str()] = &(propositions[i]);
	}


	fstream fin ("test.groups");

	int nStateVariables;
	fin >> nStateVariables;

	string line;
	getline(fin, line);

	stateVariables.resize(nStateVariables);
	for (int i = 0; i < nStateVariables; i++){
		getline(fin, line);
		int domainSize;
		fin >> domainSize;
		stateVariables[i].domain.resize(domainSize);
		stateVariables[i].variableId = i;
		getline(fin, line);
		for (int j = 0; j < domainSize; j++){
			getline(fin, line);
			size_t index = line.find("Atom ");
			if (index != string::npos){
				index += 5;   /* strlen("Atom "); */
				string theName = line.substr(index);
				stateVariables[i].domain[j] = MyStateValue (j, propositionName[theName], &(stateVariables[i]));
			}else{
				index = line.find ("<none of those>");
				if (index != string::npos){
					stateVariables[i].domain[j] = MyStateValue (j, NULL, &(stateVariables[i]));
				}else{
					CANT_HANDLE("can't handle some state value!!!!");
				}
			}
		}
	}
}


void MyProblem::updateInitialValues(){
	pc_list<assignment*>::const_iterator it = current_analysis->the_problem->initial_state->assign_effects.begin();
	pc_list<assignment*>::const_iterator itEnd = current_analysis->the_problem->initial_state->assign_effects.end();
	initialValue.resize(current_analysis->the_problem->initial_state->assign_effects.size());    //we assume in the initial state the value of every function (variable) has been declared
	FastEnvironment env(0);

	for (; it != itEnd; ++it){
		PNE pne ((*it)->getFTerm(), &env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		const num_expression *numExpr = dynamic_cast <const num_expression *>((*it)->getExpr());
		if (pne2 && numExpr && (*it)->getOp() == E_ASSIGN){
			initialValue[pne2->getGlobalID()] = numExpr->double_value();
		}else{
			CANT_HANDLE("Can't find Some Initial Value ")
		}
	}

}

void MyProblem::initializing(){

	//Preparing propositions
	int nPropositions = instantiatedOp::howManyNonStaticLiterals();
	propositions.resize(nPropositions);
	LiteralStore::iterator litIt, litItEnd;
	litIt = instantiatedOp::literalsBegin();
	litItEnd = instantiatedOp::literalsEnd();
	for (; litIt != litItEnd; ++litIt){
		if ((*litIt)->getStateID() != -1){
			propositions[(*litIt)->getStateID()].originalLiteral = *litIt;
		}
	}

	//Preparing variables
	int nVariables = instantiatedOp::howManyNonStaticPNEs();
	variables.resize(nVariables);
	PNEStore::iterator pneIt, pneItEnd;
	pneIt = instantiatedOp::pnesBegin();
	pneItEnd = instantiatedOp::pnesEnd();
	for (; pneIt != pneItEnd; ++pneIt){
		if ((*pneIt)->getStateID() != -1){
			variables[(*pneIt)->getStateID()].originalPNE = *pneIt;
		}
	}

	updateInitialValues();

	readingSASPlusFile();
	buildingDTG();



	//Preparing actions
	int nAction = instantiatedOp::howMany();
	actions.resize(nAction);
	for (int i = 0; i < nAction; i++){
		actions[i].initialize(instantiatedOp::getInstOp(i));
	}

	filterVariables();

	for (int i = 0; i < nAction; i++){
		actions[i].computeStaticMutex();
	}

}

void MyProblem::print(){

	cout << "Propositions: " << instantiatedOp::howManyNonStaticLiterals() << endl;
	for (unsigned int i = 0; i < propositions.size(); i++){
		cout << i << ' ' << propositions[i].originalLiteral->getStateID() << ' ';
		propositions[i].originalLiteral->write(cout);
//		cout << "---> " << propositions[i].stateValue->theStateVariable->variableId << ", " << propositions[i].stateValue->valueId;
		cout << endl;
	}

	cout << "State variables: " << stateVariables.size() << endl;
	for (unsigned int i = 0; i < stateVariables.size(); i++){
		cout << i << ' ' << stateVariables[i].domain.size() << endl;
		for (unsigned int j = 0; j < stateVariables[i].domain.size(); j++){
			cout << "   " << j << ": ";
			if (stateVariables[i].domain[j].theProposition != NULL){
				stateVariables[i].domain[j].theProposition->originalLiteral->write(cout);
			}else{
				cout << "<none of those>" << endl;
			}
			cout << endl;
		}
		cout << endl;
	}

	cout << "Variables: " << instantiatedOp::howManyNonStaticPNEs() << endl;
	for (unsigned int i = 0; i < variables.size(); i++){
		cout << i << ' ' << variables[i].originalPNE->getStateID() << ' ';
		variables[i].originalPNE->write(cout);
		cout << endl;
	}
	cout << "Actions: " << instantiatedOp::howMany() << endl;
	for (unsigned int i = 0; i < actions.size(); i++){
		cout << i << ' ' << actions[i].valAction->getID() << ' ';
		actions[i].valAction->write(cout);
		cout << endl;
	}
}

MyProblem::MyProblem() {
	// TODO Auto-generated constructor stub

}

MyProblem::~MyProblem() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
