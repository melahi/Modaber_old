#ifndef MYTYPE_H_
#define MYTYPE_H_

#include "VALfiles/parsing/ptree.h"
#include <list>
#include <vector>

using namespace std;
using namespace VAL;


namespace mdbr {

class MyObject;


class MyType {
public:

	bool completedChildren;

	VAL::pddl_type *originalType;

	list <MyType *> children;
	vector <MyObject *> objects;

	MyType(): completedChildren(false), originalType(NULL) {}
	virtual ~MyType() {}

	void completingChildren();
};

class MyObject {
public:

	VAL::const_symbol *originalObject;

	MyType * type;
};

} /* namespace mdbr */
#endif /* MYTYPE_H_ */
