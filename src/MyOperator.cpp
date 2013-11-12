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



list <MyPartialOperator *>::iterator MyOperator::findPartialOperator (const MyPartialOperator *a){
	list <MyPartialOperator *>::iterator ret, pAEnd;
	ret = partialOperator.begin();
	pAEnd = partialOperator.end();

	for (; ret != pAEnd; ++ret){
		if (**ret == *a){
			break;
		}
	}
	return ret;
}

void MyOperator::prepare (operator_ *originalOperator, int id){
	this->id = id;
	this->originalOperator = originalOperator;
	var_symbol_list::iterator it, itEnd;
	it = originalOperator->parameters->begin();
	itEnd = originalOperator->parameters->end();
	int nArgument = originalOperator->parameters->size();
	offset.resize(nArgument);
	argument.resize(nArgument);
	for (int i = 0; it != itEnd; ++it, ++i){
		argument[i] = &(myProblem.types[(*it)->type]);
	}

	preparePreconditions(originalOperator->precondition);
	prepareSimpleEffect(originalOperator->effects->add_effects, true);
	prepareSimpleEffect(originalOperator->effects->del_effects, false);
	prepareAssignment(originalOperator->effects->assign_effects);

	list <MyPartialOperator *>::iterator opIt, opItEnd;
	opIt = partialOperator.begin();
	opItEnd = partialOperator.end();
	for (; opIt != opItEnd; ++opIt){
		(*opIt)->grounding();
	}
}


void MyOperator::prepareSimpleEffect(pc_list<simple_effect*> &valEffectList, bool addEffect){
	pc_list<simple_effect *>::iterator it, itEnd;
	it = valEffectList.begin();
	itEnd = valEffectList.end();
	for (int i = 0; it != itEnd; ++it, ++i){
		MyPartialOperator *a = new MyPartialOperator();
		a->prepare(this, (*it)->prop);
		list <MyPartialOperator *>::iterator it2 = findPartialOperator(a);
		if (it2 == partialOperator.end()){
			partialOperator.push_back(a);
			it2 = partialOperator.end();
			it2--;
		}else{
			delete (a);
		}
		if (addEffect){
			(*it2)->addEffect.push_back((*it)->prop);
		}else{
			(*it2)->deleteEffect.push_back((*it)->prop);
		}
	}
}

void MyOperator::prepareAssignment(pc_list <assignment *> &assignmentList){
	pc_list<assignment *>::iterator it, itEnd;
	it = assignmentList.begin();
	itEnd = assignmentList.end();
	for (int i = 0; it != itEnd; ++it, ++i){
		MyPartialOperator *a = new MyPartialOperator();
		a->prepare(this, *it);
		list <MyPartialOperator *>::iterator it2 = findPartialOperator(a);
		if (it2 == partialOperator.end()){
			partialOperator.push_back(a);
			it2 = partialOperator.end();
			it2--;
		}else{
			delete (a);
		}
		(*it2)->assignmentEffect.push_back(*it);
	}
}

void MyOperator::preparePreconditions(goal *gl){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		MyPartialOperator *a = new MyPartialOperator();
		a->prepare(this, simple->getProp());
		list <MyPartialOperator *>::iterator it2 = findPartialOperator(a);
		if (it2 == partialOperator.end()){
			partialOperator.push_back(a);
			it2 = partialOperator.end();
			it2--;
		}else{
			delete (a);
		}
		(*it2)->precondition.push_back(simple->getProp());
		return;
	}
	const VAL::comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		MyPartialOperator *a = new MyPartialOperator();
		a->prepare(this, comp);
		list <MyPartialOperator *>::iterator it2 = findPartialOperator(a);
		if (it2 == partialOperator.end()){
			partialOperator.push_back(a);
			it2 = partialOperator.end();
			it2--;
		}else{
			delete (a);
		}
		(*it2)->comparisonPrecondition.push_back(comp);
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
	CANT_HANDLE("Can't handle some precondition in preparing operator!!!")
	return;
}


MyOperator::~MyOperator() {
	list <MyPartialOperator *>::iterator it, itEnd;
	it = partialOperator.begin();
	itEnd = partialOperator.end();
	for (; it != itEnd; ++it){
		delete (*it);
	}
}

} /* namespace mdbr */
