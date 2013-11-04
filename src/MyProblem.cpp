
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
					if (!isVisited(stateVariables[j].domain[addStateValue[j]].firstVisitedLayer, actions[i].firstVisitedLayer + 1)){
						stateVariables[j].domain[addStateValue[j]].firstVisitedLayer = actions[i].firstVisitedLayer + 1;
					}
				}else { /* if addStateValue[j] == -1 means the action change the corresponding state value to the "<none of those>" state value */
					stateVariables[j].domain[stateVariables[j].domain.size() - 1].providers[deleteStateValue[j]].push_back(&actions[i]);
					if (!isVisited(stateVariables[j].domain[stateVariables[j].domain.size() - 1].firstVisitedLayer, actions[i].firstVisitedLayer + 1)){
						stateVariables[j].domain[addStateValue[j]].firstVisitedLayer = actions[i].firstVisitedLayer + 1;
					}
				}
			}else if (addStateValue[j] != -1){
				stateVariables[j].domain[addStateValue[j]].providers[stateVariables[j].domain.size() - 1].push_back(&actions[i]);
				if (!isVisited(stateVariables[j].domain[addStateValue[j]].firstVisitedLayer, actions[i].firstVisitedLayer + 1)){
					stateVariables[j].domain[addStateValue[j]].firstVisitedLayer = actions[i].firstVisitedLayer + 1;
				}
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
				stateVariables[i].domain[j].initialize(j, propositionName[theName], &(stateVariables[i]));
			}else{
				index = line.find ("<none of those>");
				if (index != string::npos){
					stateVariables[i].domain[j].initialize(j, NULL, &(stateVariables[i]));
				}else{
					CANT_HANDLE("can't handle some state value!!!!");
				}
			}
		}
	}
}

void MyProblem::updateGoalValues (){

	goalValue.resize(stateVariables.size(), NULL);

	FastEnvironment env(0);
	updateGoalValues(current_analysis->the_problem->the_goal, &env);

}
void MyProblem::updateGoalValues (goal *the_goal, FastEnvironment *env){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(the_goal);
	if (simple){

		Literal lit (simple->getProp(), env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);

		if (!lit2){
			CANT_HANDLE("Warning: can't find some literal in creating state values!!!");
			lit2->write(cerr);
			return;
		}

		if (lit2->getStateID() == -1){
			return;
		}

		MyStateValue *stateValue = myProblem.propositions[lit2->getStateID()].stateValue;
		goalValue[stateValue->theStateVariable->variableId] = stateValue;
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(the_goal);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			updateGoalValues(*it, env);
		}
		return;
	}
	CANT_HANDLE("Can't create some state value from some goals!!!");
}

void MyProblem::updateInitialValuesForLiftedProposition(){
	//Find initial value for
	pc_list<simple_effect*>::const_iterator it1 = current_analysis->the_problem->initial_state->add_effects.begin();
	pc_list<simple_effect*>::const_iterator it1End = current_analysis->the_problem->initial_state->add_effects.end();
	FastEnvironment env(0);

	for (; it1 != it1End; it1++){
		Literal lit ((*it1)->prop, &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() != -1){
			liftedPropositions[lit2->getStateID()].initialValue = true;
		}
	}
}

void MyProblem::updateInitialValuesForVariables(){
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
			initialValue[pne2->getGlobalID()] = numExpr->double_value();
		}else{
			CANT_HANDLE("Can't find Some Initial Value ");
		}
	}

}

void MyProblem::initializing(bool usingSASPlus){

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

	updateInitialValuesForVariables();

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

	this->usingSASPlus = usingSASPlus;
	if (usingSASPlus){
		readingSASPlusFile();
		buildingDTG();
		updateGoalValues();
	}

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
			CANT_HANDLE("We don't support for No Type or Either Type!!!");
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
	liftedPropositions.resize(instantiatedOp::howManyNonStaticLiterals());
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

	assignIdToValues();
	assignIdToLiftedPropositions();
	list <MyAssignment>::iterator asgnIt, asgnItEnd;
	asgnIt = assignments.begin();
	asgnItEnd = assignments.end();
	for (; asgnIt != asgnItEnd; ++asgnIt){
		asgnIt->findAllMutexes();
	}

	updateInitialValuesForLiftedProposition();
}


void MyProblem::assignIdToLiftedPropositions(){
	map <Literal *, MyLiftedProposition>::iterator it, itEnd;
	it = liftedPropositions.begin();
	itEnd = liftedPropositions.end();
	nPropositionVariables = instantiatedOp::howManyLiteralsOfAnySort();

	int nOperators = operators.size();

	it = liftedPropositions.begin();
	for (; it != itEnd; ++it){
		if (it->first == NULL){
			continue;
		}

		vector <bool> possibleModificationByOperator (nOperators, false);

		//find which operator affect on this proposition
		list <MyPartialAction *>::iterator paIt, paItEnd;

		paIt = it->second.adder.begin();
		paItEnd = it->second.adder.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->op->id] = true;
		}

		paIt = it->second.deleter.begin();
		paItEnd = it->second.deleter.end();
		for (; paIt != paItEnd; ++paIt){
			possibleModificationByOperator[(*paIt)->op->id] = true;
		}



		//assigning id
		it->second.ids.resize(nOperators);
		int lastModifierOperator;
		for (lastModifierOperator = nOperators - 1; lastModifierOperator >= 0; --lastModifierOperator){
			if (possibleModificationByOperator[lastModifierOperator]){
				break;
			}
		}
		if (lastModifierOperator == -1){
			it->second.ids[0] = -1;
		}else{
			it->second.ids[0] = it->first->getGlobalID();
		}
		for (int i = 1; i < nOperators; ++i){
			if (i > lastModifierOperator){
				it->second.ids[i] = -1;
				continue;
			}
			if (possibleModificationByOperator[i - 1]){
				it->second.ids[i] = nPropositionVariables;
				nPropositionVariables++;
				continue;
			}
			it->second.ids[i] = it->second.ids[i - 1];
		}
	}
}

void MyProblem::assignIdToValues(){
	int nVariables = variables.size();
	int nOperators = operators.size();
	nValues = 0;
	for (int i = 0; i < nVariables; ++i){
		if (variables[i].domain.size() == 0){
			continue;
		}
		vector <bool> possibleModificationByOperator (nOperators, false);

		//find which operator affect on this proposition
		list <MyAssignment *>::iterator asgnIt, asgnItEnd;
		asgnIt = variables[i].assigner.begin();
		asgnItEnd = variables[i].assigner.end();
		for (; asgnIt != asgnItEnd; ++asgnIt){
			possibleModificationByOperator[(*asgnIt)->op->id] = true;
		}

		//assigning id
		int lastModifierOperator;
		for (lastModifierOperator = nOperators - 1; lastModifierOperator >= 0; --lastModifierOperator){
			if (possibleModificationByOperator[lastModifierOperator]){
				break;
			}
		}
		if (lastModifierOperator == -1){
			CANT_HANDLE("SOME THING STRANGE HAS HAPPENED (I THOUGH THIS VARIABLE IS NOT STATIC BUT NO ACTION AFFECT ON IT)");
		}

		map <double, MyValue>::iterator valueIt, valueItEnd;
		valueIt = variables[i].domain.begin();
		valueItEnd = variables[i].domain.end();
		for (; valueIt != valueItEnd; ++valueIt){
			valueIt->second.ids.resize(nOperators);
			valueIt->second.ids[0] = nValues;
//			valueIt->second.write(cout); cout << "Operator: " << 0 << " ==> id: " << nValues << endl;
			nValues++;
			for (int j = 1; j < nOperators; ++j){
				if (j > lastModifierOperator){
					valueIt->second.ids[j] = -1;
					continue;
				}
				if (possibleModificationByOperator[j - 1]){
					valueIt->second.ids[j] = nValues;
//					valueIt->second.write(cout); cout << "Operator: " << j << " ==> id: " << nValues << endl;
					nValues++;
					continue;
				}
				valueIt->second.ids[j] = valueIt->second.ids[j - 1];
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

	sout << "State variables: " << stateVariables.size() << endl;
	for (unsigned int i = 0; i < stateVariables.size(); i++){
		stateVariables[i].write(sout);
		sout << endl;
	}

	sout << "Variables: " << instantiatedOp::howManyNonStaticPNEs() << endl;
	for (unsigned int i = 0; i < variables.size(); i++){
		sout << i << ' ' << variables[i].originalPNE->getStateID() << ' ';
		variables[i].originalPNE->write(sout);
		sout << endl;
	}
	sout << "Actions: " << instantiatedOp::howMany() << endl;
	for (unsigned int i = 0; i < actions.size(); i++){
		sout << i << ' ' << actions[i].valAction->getID() << ' ';
		actions[i].valAction->write(sout);
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

void MyProblem::writeDTG(ostream &sout){
	int nStateVariables = stateVariables.size();
	for (int i = 0; i < nStateVariables; ++i){
		int domainSize = stateVariables[i].domain.size();
		sout << "Variable: " << i << endl;
		for (int j = 0; j < domainSize; ++j){
			for (int k = 0; k < domainSize; ++k){
				sout << "(" << j << ',' << k << ")  ==>  ";
				list <MyAction*>::iterator it, itEnd;
				it = stateVariables[i].domain[j].providers[k].begin();
				itEnd = stateVariables[i].domain[j].providers[k].end();
				for (; it != itEnd; ++it){
					(*it)->write(sout);
				}
				sout << endl;
			}
		}
		sout << "--------------------------------------------------------" << endl;
	}
}

void MyProblem::writeAllLiftedPropositional(){
	map<Literal *, MyLiftedProposition>::iterator it, itEnd;
	it = liftedPropositions.begin();
	itEnd = liftedPropositions.end();

	for (; it != itEnd; ++it){
		it->second.write(cout);
	}

}

MyProblem::MyProblem() {
	// TODO Auto-generated constructor stub

}

} /* namespace mdbr */
