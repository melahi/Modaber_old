/*
 * MyPartialAction.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: sadra
 */

#include "MyPartialAction.h"
#include "MyLiftedProposition.h"

#include "MyProblem.h"

#include <vector>
using namespace std;

namespace mdbr {
//
//MyPartialAction::MyPartialAction(propositionKind pKind, MyOperator *op, MyPredicate *predicate, vector <MyObject*> &argument, vector <int> &objectId, int id) {
//	prepare (pKind, op, predicate, argument, objectId, id);
//}

void MyPartialAction::prepare (propositionKind pKind, MyOperator *op, MyLiftedPartialAction *predicate, vector <MyObject*> &argument, vector <int> &objectId, int id){
	this->id = id;
	this->pKind = pKind;
	this->op = op;
	this->liftedPartialAction = predicate;
	MyLiftedProposition prop(predicate->originalPredicate, argument);
	this->proposition = prop.find();

	int nObjects = objectId.size();
	this->objectId.resize(nObjects);
	for (int i = 0; i < nObjects; i++){
		this->objectId[i] = objectId[i];
	}

	if (pKind == mdbr::precondition){
		this->proposition->needer.push_back(this);
	}else if (pKind == mdbr::addEffect){
		this->proposition->adder.push_back(this);
	}else if (pKind == mdbr::deleteEffect){
		this->proposition->deleter.push_back(this);
	}
}

bool MyPartialAction::isForSameAction (MyPartialAction *other){
	if (proposition->originalLiteral->getGlobalID() != other->proposition->originalLiteral->getGlobalID()){
		return false;
	}
	if (op->id != other->op->id){
		return false;
	}
	int nArgument = liftedPartialAction->argument.size();
	for (int i = 0; i < nArgument; i++){
		if (liftedPartialAction->placement[i] != other->liftedPartialAction->placement[i]){
			return false;
		}
	}
	return true;
}


void MyPartialAction::write(ostream &sout, bool isEndl /* = true */){
	sout << "[ " << this->id << " ";
	if (pKind == precondition){
		sout << "PRE";
	}else if (pKind == addEffect){
		sout << "ADD";
	}else if (pKind == deleteEffect){
		sout << "DEL";
	}else {
		sout << "SOMETHING ELSE!!!!";
	}
	sout << "(";
	sout << this->op->originalOperator->name->getName();
	int nArgument = this->liftedPartialAction->argument.size();
	for (int i = 0; i < nArgument; i++){
		sout << " " << this->liftedPartialAction->placement[i] << ":" << this->liftedPartialAction->argument[i]->objects[this->objectId[i]]->originalObject->getName();
	}
	sout <<") ==> ";
	if (this->proposition->originalLiteral){
		this->proposition->originalLiteral->write(sout);
	}else{
		cout << "FALSE!!!";
	}
	sout << "]";
	if (isEndl){
		sout << endl;
	}
}

MyPartialAction::~MyPartialAction() {
}

} /* namespace mdbr */
