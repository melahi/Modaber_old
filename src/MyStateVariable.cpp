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

MyStateVariable::~MyStateVariable() {
	// TODO Auto-generated destructor stub
}

MyStateValue::MyStateValue(int valueId, MyProposition *theProposition, MyStateVariable *theStateVariable):valueId(valueId),  theProposition(theProposition), theStateVariable(theStateVariable), providers(theStateVariable->domain.size()) {
	if (theProposition != NULL){
		theProposition->stateValue = this;
	}
}

MyStateValue::MyStateValue():valueId(-1), theProposition(0), theStateVariable(0){}


} /* namespace mdbr */
