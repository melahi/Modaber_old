/*
 * MyLiftedProposition.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#include "MyLiftedProposition.h"
#include "MyProblem.h"
#include "Utilities.h"

namespace mdbr {


void MyLiftedPartialAction::prepare(MyOperator *op, propositionKind pKind,const proposition *prop){
	originalPredicate = prop;
	this->op =  op;
	this->pKind = pKind;
	nArguments = prop->args->size();
	argument.resize(nArguments);
	placement.resize(nArguments);
	parameter_symbol_list::iterator pIt, pItEnd;
	pIt = prop->args->begin();
	pItEnd = prop->args->end();
	for (int i = 0; pIt != pItEnd; ++pIt, ++i){
		var_symbol_list::iterator it, itEnd;
		argument[i] = &(myProblem.types[(*pIt)->type]);
		it = op->originalOperator->parameters->begin();
		itEnd = op->originalOperator->parameters->end();
		for (int j = 0; it != itEnd; ++it, ++j){
			if ((*it)->getName() == (*pIt)->getName()){
				placement[i] = j;
				break;
			}
		}
	}

	grounding();
}

void MyLiftedPartialAction::grounding() {
	if (grounded){
		return;
	}
	selectedObject.resize(argument.size());
	IdOfSelectedObject.resize(argument.size());
	grounding(0);
	grounded = true;
}

void MyLiftedPartialAction::grounding(unsigned int argumentIndex){
	if (argumentIndex == argument.size()){
		int partialActionId = myProblem.partialAction.size();
		myProblem.partialAction.push_back(MyPartialAction());
		myProblem.partialAction.rbegin()->prepare(pKind, op, this, selectedObject, IdOfSelectedObject, partialActionId);
		return;
	}
	int nObjects = argument[argumentIndex]->objects.size();
	for (int i = 0; i < nObjects; ++i){
		selectedObject[argumentIndex] = argument[argumentIndex]->objects[i];
		IdOfSelectedObject [argumentIndex] = i;
		grounding(argumentIndex + 1);
	}
}


MyLiftedProposition::MyLiftedProposition(const proposition *valProposition, vector <MyObject *> &arguments){
	this->initialValue = false;

	parameter_symbol_list *parameters = new parameter_symbol_list;
	int nArgs = arguments.size();
	this->arguments.resize(nArgs);
	for (int i = 0; i < nArgs; ++i){
		parameters->push_back(arguments[i]->originalObject);
		this->arguments[i] = arguments[i];
	}

	proposition prop(valProposition->head, parameters);
	FastEnvironment env(0);
	Literal lit(&prop, &env);
	originalLiteral = instantiatedOp::findLiteral(&lit);
}

MyLiftedProposition *MyLiftedProposition::find(){
	map <Literal *, MyLiftedProposition>::iterator it;
	it = myProblem.liftedPropositions.find(this->originalLiteral);
	if (it == myProblem.liftedPropositions.end()){
		myProblem.liftedPropositions[this->originalLiteral] = *this ;
		return &(myProblem.liftedPropositions[this->originalLiteral]);
	}
	return &(it->second);
}

void MyLiftedProposition::write (ostream &sout){
	//I think we don't need this function anymore, if you are disagree with me the correct it by yourself
//	originalLiteral->write(sout);
//	sout << " -> " << id << endl;
//
//	sout << "NEEDER: " << endl;
//	list <MyPartialAction *>::iterator it, itEnd;
//	it = needer.begin();
//	itEnd = needer.end();
//	for (; it != itEnd; ++it){
//		sout << (*it)->id << ' ';
//	}
//	sout << endl;
//
//	sout << "ADDER: " << endl;
//	it = adder.begin();
//	itEnd = adder.end();
//	for (; it != itEnd; ++it){
//		sout << (*it)->id << ' ';
//	}
//	sout << endl;
//
//	sout << "DELETER: " << endl;
//	it = deleter.begin();
//	itEnd = deleter.end();
//	for (; it != itEnd; ++it){
//		sout << (*it)->id << ' ';
//	}
//	sout << endl;
}

MyLiftedProposition::~MyLiftedProposition() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
