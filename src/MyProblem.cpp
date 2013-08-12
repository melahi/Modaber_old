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


using namespace VAL;
using namespace Inst;

namespace mdbr {

MyProblem myProblem;

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

	//Preparing actions
	int nAction = instantiatedOp::howMany();
	actions.resize(nAction);
	for (int i = 0; i < nAction; i++){
		actions[i].initialize(instantiatedOp::getInstOp(i));
	}

	for (int i = 0; i < nAction; i++){
		actions[i].computeStaticMutex();
	}

	updateInitialValues();

}

void MyProblem::print(){

	cout << "Propositions: " << instantiatedOp::howManyNonStaticLiterals() << endl;
	for (unsigned int i = 0; i < propositions.size(); i++){
		cout << i << ' ' << propositions[i].originalLiteral->getStateID() << ' ';
		propositions[i].originalLiteral->write(cout);
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
