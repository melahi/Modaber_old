

#ifndef UNRELATEDFILTER_H_
#define UNRELATEDFILTER_H_

#include "VALfiles/parsing/ptree.h"
#include "VALfiles/FastEnvironment.h"
#include "VALfiles/instantiation.h"

using namespace VAL;
using namespace Inst;

namespace mdbr {

class UnrelatedFilter {
	/*This class filters actions and propositions which are unrelated to the goals */

	void considerAsEffective (const goal *gl, FastEnvironment *env);

	bool canBeEffective (instantiatedOp *op);

public:
	UnrelatedFilter();
	virtual ~UnrelatedFilter();
};

} /* namespace mdbr */
#endif /* UNRELATEDFILTER_H_ */
