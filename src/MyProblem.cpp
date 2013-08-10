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

MyProblem::MyProblem() {
	// TODO Auto-generated constructor stub

}

MyProblem::~MyProblem() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
