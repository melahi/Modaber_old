
#include "UnrelatedFilter.h"

#include "MyProblem.h"

#include "VALfiles/instantiation.h"

using namespace Inst;



namespace mdbr {


void UnrelatedFilter::considerAsEffective (const goal *gl, FastEnvironment *env){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);

	if (simple){

		Literal lit(simple->getProp(), env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);

		if (lit2->getStateID() == -1){
			return ;
		}

		myProblem.propositions[lit2->getStateID()].possibleEffective = true;
		return;

	}

	const comparison *comp = dynamic_cast<const comparison*> (gl);

	if (comp){
		return;
	}

	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			considerAsEffective(*it, env);
		}
		return;
	}
	const preference *pref = dynamic_cast <const preference *> (gl);
	if (pref){
		considerAsEffective(pref->getGoal(), env);
		return;
	}
	CANT_HANDLE("can't evaluate some precondition");
	return;
}


bool UnrelatedFilter::canBeEffective (instantiatedOp *action){
	pc_list <assignment *>::const_iterator it, itEnd;
	FOR_ITERATION(it, itEnd, action->forOp()->effects->assign_effects){
		if ((*it)->getFTerm()->getFunction()->getName() != "total-cost"){
			//Because perhaps this action do something good for some variables but for now we
			//can't analyze numerical variables, so we relax this case and consider this action
			//can be effective
			return true;
		}
	}
	pc_list<simple_effect*>::const_iterator it2, it2End;
	FOR_ITERATION(it2, it2End, action->forOp()->effects->add_effects) {
		Literal lit ((*it2)->prop, action->getEnv());
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		if (myProblem.propositions[lit2->getStateID()].possibleEffective){
			return true;
		}
	}
	return false;
}

UnrelatedFilter::UnrelatedFilter() {


	int nOperators = current_analysis->the_domain->ops->size();

	bool canContinue = true;
	FastEnvironment env(0);
	considerAsEffective(current_analysis->the_problem->the_goal, &env);

	while (canContinue){
		canContinue = false;
		for (int i = 0; i < nOperators; ++i){
			int nActions = myProblem.actions[i].size();
			if (nActions == 0){
				continue;
			}
			const goal *gl = myProblem.actions[i][0]->valAction->forOp()->precondition;
			for (int j = 0; j < nActions; ++j){
				if ((!myProblem.actions[i][j]->possibleEffective) && canBeEffective(myProblem.actions[i][j]->valAction)){
					myProblem.actions[i][j]->possibleEffective = true;
					considerAsEffective(gl, myProblem.actions[i][j]->valAction->getEnv());
					canContinue = true;
				}
			}
		}
	}
}

UnrelatedFilter::~UnrelatedFilter() {
}

} /* namespace mdbr */
