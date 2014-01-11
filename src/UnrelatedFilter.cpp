
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
	considerAsEffective(pref->getGoal(), env);
	CANT_HANDLE("can't evaluate some precondition");
	return;
}


bool UnrelatedFilter::canBeEffective (const pc_list <simple_effect*> *addEffect, FastEnvironment *env){
	pc_list<simple_effect*>::const_iterator it = addEffect->begin();
	pc_list<simple_effect*>::const_iterator itEnd = addEffect->end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		if (myProblem.propositions[lit2->getStateID()].possibleEffective){
			return true;
		}
	}
	return false;
}

UnrelatedFilter::UnrelatedFilter() {


	int nOperators = myProblem.operators.size();

	bool canContinue = true;
	FastEnvironment env(0);
	considerAsEffective(current_analysis->the_problem->the_goal, &env);

	while (canContinue){
		for (int i = 0; i < nOperators; ++i){
			int nActions = myProblem.actions[i].size();
			const pc_list <simple_effect*> *addEffect = &(myProblem.operators[i]->originalOperator->effects->add_effects);
			const goal *gl = myProblem.operators[i]->originalOperator->precondition;
			for (int j = 0; j < nActions; ++j){
				if ((!myProblem.actions[i][j].possibleEffective) && canBeEffective(addEffect, myProblem.actions[i][j].valAction->getEnv())){
					myProblem.actions[i][j].possibleEffective = true;
					considerAsEffective(gl, myProblem.actions[i][j].valAction->getEnv());
					canContinue = true;
				}
			}
		}
	}
}

UnrelatedFilter::~UnrelatedFilter() {
}

} /* namespace mdbr */
