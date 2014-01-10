
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
		actions[operatorMap[valAction->getHead()->getName()]].push_back(MyAction());
		actions[operatorMap[valAction->getHead()->getName()]].rbegin()->initialize(valAction);
	}


	updateInitialLayer();

//	write(cout);
}

void MyProblem::liftedInitializing(){
	//Preparing types;
	types.clear();
	typed_symbol_list<VAL::pddl_type>::iterator typeIt, typeItEnd;
	typeIt = current_analysis->the_domain->types->begin();
	typeItEnd = current_analysis->the_domain->types->end();
	for (; typeIt != typeItEnd; ++typeIt){
		types[(*typeIt)].originalType = (*typeIt);
		if ((*typeIt)->type){
			types[(*typeIt)->type].originalType = (*typeIt)->type;
			types[(*typeIt)->type].children.push_back(&types[(*typeIt)]);
		}
		if ((*typeIt)->either_types){
			CANT_HANDLE("I don't know what does either types means in type class!!!");
		}
	}


	//Preparing object
	const_symbol_list::iterator objIt, objItEnd;
	objIt = current_analysis->the_problem->objects->begin();
	objItEnd = current_analysis->the_problem->objects->end();

	for (; objIt != objItEnd; ++objIt){
		objects[(*objIt)].originalObject = (*objIt);
		MyObject *myObject = (&objects[(*objIt)]);
		if ((*objIt)->type){
			types[(*objIt)->type].objects.push_back(myObject);
			myObject->type = &types[(*objIt)->type];
		}else {
			CANT_HANDLE("We don't support  \"No Type\" or \"Either Type\"!!!");
		}
	}
	//Also we should consider constants
	if (current_analysis->the_domain->constants){
		objIt = current_analysis->the_domain->constants->begin();
		objItEnd = current_analysis->the_domain->constants->end();

		for (; objIt != objItEnd; ++objIt){
			objects[(*objIt)].originalObject = (*objIt);
			MyObject *myObject = (&objects[(*objIt)]);
			if ((*objIt)->type){
				types[(*objIt)->type].objects.push_back(myObject);
				myObject->type = &types[(*objIt)->type];
			}else {
				CANT_HANDLE("We don't support  \"No Type\" or \"Either Type\"!!!");
			}
		}
	}


	//completing objects of each type;
	map<VAL::pddl_type *, MyType>::iterator myTypeIt, myTypeItEnd;
	myTypeIt = types.begin();
	myTypeItEnd = types.end();

	for (; myTypeIt != myTypeItEnd; ++myTypeIt){
		myTypeIt->second.completingChildren();
	}


	//preparing operators
	nPartialActions = 0;
	operators.resize(current_analysis->the_domain->ops->size());
	operator_list::iterator opIt, opItEnd;
	opIt = current_analysis->the_domain->ops->begin();
	opItEnd = current_analysis->the_domain->ops->end();
	nUnification = 0;
	for (int i = 0; opIt != opItEnd; ++opIt, ++i){
		operators[i] = new MyOperator();
		operators[i]->prepare(*opIt, i);
		for (unsigned int j = 0; j < operators[i]->offset.size(); ++j){
			operators[i]->offset[j] = nUnification;
			nUnification += operators[i]->argument[j]->objects.size();
		}
	}

	assignIdToPropositions();
	assignIdToVariables();
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
			possibleModificationByOperator[(*paIt)->op->id] = true;
		}

		paIt = it->deleter.begin();
		paItEnd = it->deleter.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->op->id] = true;
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
			possibleModificationByOperator[(*paIt)->op->id] = true;
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

	//Printing Types
	map <VAL::pddl_type *, MyType>::iterator typeIt, typeItEnd;
	typeIt = types.begin();
	typeItEnd = types.end();
	for (; typeIt != typeItEnd; ++typeIt){
		writeType(sout, &(typeIt->second), 0);
	}
}

void MyProblem::writeType (ostream &sout, MyType *type, int indent){
	string myIndent = "";
	for (int i = 0; i < indent; i++){
		myIndent += '\t';
	}

	sout << myIndent << type->originalType->getName() << ": " << endl;
	vector <MyObject *>::iterator objIt, objItEnd;
	objIt = type->objects.begin();
	objItEnd = type->objects.end();
	for (; objIt != objItEnd; ++objIt){
		sout << myIndent << "---" << (*objIt)->originalObject->getName() << endl;
	}

	list <MyType *>::iterator typeIt, typeItEnd;
	typeIt = type->children.begin();
	typeItEnd = type->children.end();

	for (; typeIt != typeItEnd; ++typeIt){
		writeType(sout, *typeIt, indent+1);
	}
}


} /* namespace mdbr */
