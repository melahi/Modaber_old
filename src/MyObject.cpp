/*
 * MyType.cpp
 *
 *  Created on: Sep 22, 2013
 *      Author: sadra
 */

#include "MyObject.h"

namespace mdbr {

void MyType::completingChildren(){
	if (completedChildren){
		return;
	}
	list <MyType *>::iterator it, itEnd;
	it = children.begin();
	itEnd = children.end();
	for (; it != itEnd; ++it){
		(*it)->completingChildren();
		vector <MyObject *>::iterator objIt, objItEnd;
		objIt = (*it)->objects.begin();
		objItEnd = (*it)->objects.end();
		for (; objIt != objItEnd; ++objIt){
			objects.push_back(*objIt);
		}
	}
	completedChildren = true;
}
} /* namespace mdbr */
