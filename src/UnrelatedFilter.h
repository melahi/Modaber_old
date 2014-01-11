

#ifndef UNRELATEDFILTER_H_
#define UNRELATEDFILTER_H_

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"

using namespace VAL;

namespace mdbr {

class UnrelatedFilter {
	/*This class filters actions and propositions which are unrelated to the goals */

	void considerAsEffective (const goal *gl, FastEnvironment *env);

	bool canBeEffective (const pc_list <simple_effect*> *addEffect, FastEnvironment *env);

public:
	UnrelatedFilter();
	virtual ~UnrelatedFilter();
};

} /* namespace mdbr */
#endif /* UNRELATEDFILTER_H_ */
