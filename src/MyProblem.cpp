
#include "MyProblem.h"
#include "MyAction.h"
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

void MyProblem::updateInitialLayer(){

	//Find initial value for variables
	pc_list<assignment*>::const_iterator it = current_analysis->the_problem->initial_state->assign_effects.begin();
	pc_list<assignment*>::const_iterator itEnd = current_analysis->the_problem->initial_state->assign_effects.end();
	initialValue.resize(current_analysis->the_problem->initial_state->assign_effects.size());    //we assume in the initial state the value of every function (variable) has been declared
	FastEnvironment env(0);


	for (; it != itEnd; ++it){
		PNE pne ((*it)->getFTerm(), &env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		const num_expression *numExpr = dynamic_cast <const num_expression *>((*it)->getExpr());
		if (pne2 && numExpr && (*it)->getOp() == E_ASSIGN){
			double theInitialValue = numExpr->double_value();
			initialValue[pne2->getGlobalID()] = theInitialValue;
		}else{
			CANT_HANDLE("Can't find Some Initial Value ");
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
	map <string, int> operatorMap;

	operator_list::iterator opIt, opItEnd;

	opIt = current_analysis->the_domain->ops->begin();
	opItEnd = current_analysis->the_domain->ops->end();

	int nOperator;
	for (nOperator = 0; opIt != opItEnd; ++opIt, ++nOperator){
		operatorMap[ (*opIt)->name->getName() ] = nOperator;
	}

	actions.resize(nOperator);
	for (int i = 0; i < nAction; i++){
		instantiatedOp *valAction = instantiatedOp::getInstOp(i);
		actions[operatorMap[valAction->getHead()->getName()]].push_back(new MyAction());
		(*actions[operatorMap[valAction->getHead()->getName()]].rbegin())->initialize(valAction, i);
	}

	updateInitialLayer();

//	write(cout);
}

void MyProblem::liftedInitializing(){

	//preparing operators
	nPartialActions = 0;
	int nOperators = current_analysis->the_domain->ops->size();
	operators.resize(nOperators);
	operator_list::iterator opIt, opItEnd;
	opIt = current_analysis->the_domain->ops->begin();
	opItEnd = current_analysis->the_domain->ops->end();
	for (int i = 0; opIt != opItEnd; ++opIt, ++i){
		operators[i] = new MyOperator();
		operators[i]->prepare(*opIt, i);
	}

	//preparing partial actions

	for (int i = 0; i < nOperators; ++i){
		int nActions = actions[i].size();
		for (int j = 0; j < nActions; ++j){
			if (actions[i][j]->possibleEffective){
				operators[i]->consideringAction(actions[i][j]->valAction);
			}
		}
	}

	assignIdToPropositions();
	assignIdToVariables();
	assignIdtoUnification();
}


void MyProblem::reconsiderValues(){

	//Considering +infinity and -infinity for each variable
	int nVariables = variables.size();
	for (int i = 0; i < nVariables; ++i){
		variables[i].domain[+infinite];
		variables[i].domain[-infinite];
	}


	int nOperators = operators.size();
	for (int i = 0; i < nOperators; ++i){
		int nPartialOperators = operators[i]->partialOperator.size();
		for (int j = 0; j < nPartialOperators; ++j){
			int nPartialActions = operators[i]->partialOperator[j]->child.size();
			for (int k = 0; k < nPartialActions; ++k){
				operators[i]->partialOperator[j]->child[k]->constructNumericalCondition();
			}
		}
	}

	assignIdToValues();
}


void MyProblem::reconsiderValues_EStep(){

	//Considering +infinity and -infinity for each variable
	int nVariables = variables.size();
	for (int i = 0; i < nVariables; ++i){
		variables[i].domain[+infinite];
		variables[i].domain[-infinite];
	}


	int nOperators = actions.size();
	for (int i = 0; i < nOperators; ++i){
		int nActions = actions[i].size();
		for (int j = 0; j < nActions; ++j){
			actions[i][j]->constructNumericalCondition();
		}
	}
	assignIdToValues_EStep();
}


void MyProblem::assignIdToPropositions(){
	vector <MyProposition>::iterator it, itEnd;
	it = propositions.begin();
	itEnd = propositions.end();
	int nOperators = operators.size();

	nPropositionIDs = 0;

	for (; it != itEnd; ++it){

		vector <bool> possibleModificationByOperator (nOperators, false);

		//find which operator affect on this proposition
		list <MyPartialAction *>::iterator paIt, paItEnd;

		paIt = it->adder.begin();
		paItEnd = it->adder.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->partialOperator->op->id] = true;
		}

		paIt = it->deleter.begin();
		paItEnd = it->deleter.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->partialOperator->op->id] = true;
		}

		//assigning id
		int lastModifierOperator;
		for (lastModifierOperator = nOperators - 1; lastModifierOperator >= 0; --lastModifierOperator){
			if (possibleModificationByOperator[lastModifierOperator]){
				break;
			}
		}
		it->ids[0] = nPropositionIDs++;
		for (int i = 1; i < nOperators; ++i){
			if (i > lastModifierOperator){
				it->ids[i] = -1;
				continue;
			}
			if (possibleModificationByOperator[i - 1]){
				it->ids[i] = nPropositionIDs++;
				continue;
			}
			it->ids[i] = it->ids[i - 1];
		}
	}
}


void MyProblem::assignIdToVariables(){
	vector <MyVariable>::iterator it, itEnd;
	it = variables.begin();
	itEnd = variables.end();
	int nOperators = operators.size();

	nVariableIDs = 0;

	for (; it != itEnd; ++it){

		vector <bool> possibleModificationByOperator (nOperators, false);

		//find which operator affect on this proposition
		list <MyPartialAction*>::iterator paIt, paItEnd;

		paIt = it->modifier.begin();
		paItEnd = it->modifier.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->partialOperator->op->id] = true;
		}

		//assigning id
		int lastModifierOperator;
		for (lastModifierOperator = nOperators - 1; lastModifierOperator >= 0; --lastModifierOperator){
			if (possibleModificationByOperator[lastModifierOperator]){
				break;
			}
		}
		it->ids[0] = nVariableIDs++;
		for (int i = 1; i < nOperators; ++i){
			if (i > lastModifierOperator){
				it->ids[i] = -1;
				continue;
			}
			if (possibleModificationByOperator[i - 1]){
				it->ids[i] = nVariableIDs++;
				continue;
			}
			it->ids[i] = it->ids[i - 1];
		}
	}
}

void MyProblem::assignIdToValues(){
	vector <MyVariable>::iterator it, itEnd;
	it = variables.begin();
	itEnd = variables.end();
	int nOperators = operators.size();

	nValueIDs = 0;

	for (; it != itEnd; ++it){

		vector <bool> possibleModificationByOperator (nOperators, false);

		//find which operator affect on this proposition
		list <MyPartialAction*>::iterator paIt, paItEnd;

		paIt = it->modifier.begin();
		paItEnd = it->modifier.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->partialOperator->op->id] = true;
		}

		//assigning id
		int lastModifierOperator;
		for (lastModifierOperator = nOperators - 1; lastModifierOperator >= 0; --lastModifierOperator){
			if (possibleModificationByOperator[lastModifierOperator]){
				break;
			}
		}

		map <double, vector<int> >::iterator domainIt, domainItEnd;

		FOR_ITERATION(domainIt, domainItEnd, it->domain){
			domainIt->second.resize(nOperators);
			domainIt->second[0] = nValueIDs++;
		}
		for (int i = 1; i < nOperators; ++i){
			if (i > lastModifierOperator){
				FOR_ITERATION(domainIt, domainItEnd, it->domain){
					domainIt->second[i] = -1;
				}
				continue;
			}
			if (possibleModificationByOperator[i - 1]){
				FOR_ITERATION(domainIt, domainItEnd, it->domain){
					domainIt->second[i] = nValueIDs++;
				}
				continue;
			}
			FOR_ITERATION(domainIt, domainItEnd, it->domain){
				domainIt->second[i] = domainIt->second[i-1];
			}
		}
	}
}


void MyProblem::assignIdToPropositions_EStep(){
	vector <MyProposition>::iterator it, itEnd;
	it = propositions.begin();
	itEnd = propositions.end();
	int nActions = instantiatedOp::howMany();

	nPropositionIDs = 0;

	for (; it != itEnd; ++it){

		vector <bool> possibleModificationByAction (nActions, false);

		//find which action affect on this proposition
		vector <MyAction *>::iterator aIt, aItEnd;

		FOR_ITERATION(aIt, aItEnd, it->adder_groundAction){
			possibleModificationByAction[(*aIt)->id] = true;
		}

		FOR_ITERATION(aIt, aItEnd, it->deleter_groundAction){
			possibleModificationByAction[(*aIt)->id] = true;
		}

		//assigning id
		it->ids.resize(nActions, -2);
		int lastModifierAction;
		for (lastModifierAction = nActions - 1; lastModifierAction >= 0; --lastModifierAction){
			if (possibleModificationByAction[lastModifierAction]){
				break;
			}
		}
		it->ids[0] = nPropositionIDs++;
		for (int i = 1; i < nActions; ++i){
			if (i > lastModifierAction){
				it->ids[i] = -1;
				continue;
			}
			if (possibleModificationByAction[i - 1]){
				it->ids[i] = nPropositionIDs++;
				continue;
			}
			it->ids[i] = it->ids[i - 1];
		}
	}
}


void MyProblem::assignIdToValues_EStep(){
	vector <MyVariable>::iterator it, itEnd;
	it = variables.begin();
	itEnd = variables.end();
	int nActions = instantiatedOp::howMany();

	nValueIDs = 0;

	for (; it != itEnd; ++it){

		vector <bool> possibleModificationByAction (nActions, false);

		//find which operator affect on this proposition
		vector <MyAction*>::iterator aIt, aItEnd;

		FOR_ITERATION(aIt, aItEnd, it->modifier_groundAction){
			possibleModificationByAction[(*aIt)->id] = true;
		}

		//assigning id
		int lastModifierAction;
		for (lastModifierAction = nActions - 1; lastModifierAction >= 0; --lastModifierAction){
			if (possibleModificationByAction[lastModifierAction]){
				break;
			}
		}

		map <double, vector<int> >::iterator domainIt, domainItEnd;

		FOR_ITERATION(domainIt, domainItEnd, it->domain){
			domainIt->second.resize(nActions);
			domainIt->second[0] = nValueIDs++;
		}
		for (int i = 1; i < nActions; ++i){
			if (i > lastModifierAction){
				FOR_ITERATION(domainIt, domainItEnd, it->domain){
					domainIt->second[i] = -1;
				}
				continue;
			}
			if (possibleModificationByAction[i - 1]){
				FOR_ITERATION(domainIt, domainItEnd, it->domain){
					domainIt->second[i] = nValueIDs++;
				}
				continue;
			}
			FOR_ITERATION(domainIt, domainItEnd, it->domain){
				domainIt->second[i] = domainIt->second[i-1];
			}
		}
	}
}



void MyProblem::assignIdtoUnification() {
	nUnification = 0;

	int nOperators = operators.size();
	for (int i = 0; i < nOperators; ++i){
		operators[i]->findingUnifications();

		int nArgument = operators[i]->argument.size();
		for (int j = 0; j < nArgument; ++j){
			map <string, int>::iterator it, itEnd;
			FOR_ITERATION(it, itEnd, operators[i]->unificationId[j]){
				it->second = nUnification++;
			}
		}
	}
}


void MyProblem::write(ostream &sout){

	sout << "Propositions: " << instantiatedOp::howManyNonStaticLiterals() << endl;
	for (unsigned int i = 0; i < propositions.size(); i++){
		propositions[i].write(sout);
		sout << endl;
	}

	sout << "Variables: " << instantiatedOp::howManyNonStaticPNEs() << endl;
	for (unsigned int i = 0; i < variables.size(); i++){
		sout << i << ' ' << variables[i].originalPNE->getStateID() << ' ';
		variables[i].originalPNE->write(sout);
		sout << endl;
	}
}


} /* namespace mdbr */
