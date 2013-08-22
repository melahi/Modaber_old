/*
 * MyStateVariable.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: sadra
 */

#include "MyStateVariable.h"

namespace mdbr {

MyStateVariable::MyStateVariable() {
	// TODO Auto-generated constructor stub

}

void MyStateVariable::write(ostream &sout){
	sout << "[ state-variable: " << variableId << ", ";
	for (unsigned int i = 0; i < domain.size(); i++){
		if (i){
			sout << ", ";
		}
		domain[i].write(sout);
	}
	sout << " ]";
}

MyStateVariable::~MyStateVariable() {
	// TODO Auto-generated destructor stub
}

void MyStateValue::initialize(int valueId, MyProposition *theProposition, MyStateVariable *theStateVariable){

	this->valueId = valueId;
	this->firstVisitedLayer = -1;
	this->theProposition = theProposition;
	this->theStateVariable = theStateVariable;
	this->providers.resize(theStateVariable->domain.size());

	if (theProposition != NULL){
		theProposition->stateValue = this;
	}
}

MyStateValue::MyStateValue():valueId(-1), firstVisitedLayer(-1), theProposition(0), theStateVariable(0){}


void MyStateValue::write(ostream &sout){
	sout << "( valueId: " << valueId << " -> ";
	if (theProposition){
		theProposition->write(sout);
	}else{
		sout << "<none of those>";
	}
	sout << " )";
}

} /* namespace mdbr */
