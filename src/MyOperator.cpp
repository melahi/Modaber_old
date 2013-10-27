/*
 * MyOperator.cpp
 *
 *  Created on: Sep 22, 2013
 *      Author: sadra
 */

#include "MyOperator.h"
#include "Utilities.h"
#include "MyProblem.h"

namespace mdbr {

MyOperator::MyOperator() {}

void MyOperator::prepare (operator_ *originalOperator, int id){
	this->id = id;
	this->originalOperator = originalOperator;
	var_symbol_list::iterator it, itEnd;
	it = originalOperator->parameters->begin();
	itEnd = originalOperator->parameters->end();
	offset.resize(originalOperator->parameters->size());
	argument.resize(originalOperator->parameters->size());
	for (int i = 0; it != itEnd; ++it, ++i){
		argument[i] = &(myProblem.types[(*it)->type]);
	}
	prepareSimpleEffect(originalOperator->effects->add_effects, mdbr::addEffect, myAddEffect);
	prepareSimpleEffect(originalOperator->effects->del_effects, mdbr::deleteEffect, myDeleteEffect);
	preparePreconditions(originalOperator->precondition);
	prepareAssignment(originalOperator->effects->assign_effects);
}


void MyOperator::prepareSimpleEffect(pc_list<simple_effect*> &valEffectList, propositionKind pKind, vector <MyLiftedPartialAction *> &effectList){
	pc_list<simple_effect *>::iterator it, itEnd;
	it = valEffectList.begin();
	itEnd = valEffectList.end();
	effectList.resize(valEffectList.size());
	for (int i = 0; it != itEnd; ++it, ++i){
		effectList[i] = new MyLiftedPartialAction();
		effectList[i]->prepare(this, pKind, (*it)->prop);
	}
}

void MyOperator::prepareAssignment(pc_list <assignment *> &assignmentList){
	pc_list<assignment *>::iterator it, itEnd;
	it = assignmentList.begin();
	itEnd = assignmentList.end();
	myAssignment.resize(assignmentList.size());
	for (int i = 0; it != itEnd; ++it, ++i){
		myAssignment[i] = new MyLiftedAssignment();
		myAssignment[i]->prepare(this, *it);
	}
}

void MyOperator::preparePreconditions(goal *gl){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		myPrecondition.push_back(new MyLiftedPartialAction());
		myPrecondition[myPrecondition.size() - 1]->prepare(this , mdbr::precondition, simple->getProp());
		return;
	}
	const VAL::comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		myComparison.push_back(new MyLiftedComparison());
		(*(myComparison.rbegin()))->prepare(this, comp);
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		goal_list::const_iterator glIt, glItEnd;
		glIt = conjunctive->getGoals()->begin();
		glItEnd = conjunctive->getGoals()->end();
		for (; glIt != glItEnd; ++glIt){
			preparePreconditions(*glIt);
		}
		return;
	}
	CANT_HANDLE("Can't handle some precondition in analyzing!!!")
	return;
}


MyOperator::~MyOperator() {
	int theSize = myAddEffect.size();
	for (int i = 0; i < theSize; i++){
		delete (myAddEffect[i]);
	}
	theSize = myDeleteEffect.size();
	for (int i = 0; i < theSize; i++){
		delete (myDeleteEffect[i]);
	}
	theSize = myPrecondition.size();
	for (int i = 0; i < theSize; i++){
		delete (myPrecondition[i]);
	}
	theSize = myAssignment.size();
	for (int i = 0; i < theSize; i++){
		delete (myAssignment[i]);
	}
	theSize = myComparison.size();
	for (int i = 0; i < theSize; i++){
		delete (myComparison[i]);
	}
}

} /* namespace mdbr */
