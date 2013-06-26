

#include "SketchyPlan.h"
#include "Utilities.h"
#include "NumericRPG.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/parsing/ptree.h"
#include <vector>
#include <set>



using namespace std;
using namespace VAL;
using namespace Inst;

SketchyPlan::SketchyPlan(NumericRPG *numericRPG): numericRPG(numericRPG) {
	propositionSelectionRatio = 1.0 / 10.0;
	fitness = -1;
	createRandomSketchyPlan();
}

void SketchyPlan::createRandomSketchyPlan() {
	int length = numericRPG->minimumPlanLength + (10 * drand48());
	milestones.resize(length);

	LiteralStore::iterator it, itEnd;
	itEnd = instantiatedOp::literalsEnd();

	for (int i = 1; i < length; i += 2){
		it = instantiatedOp::literalsBegin();
		for (;it != itEnd; it++){
			if ((*it)->getStateID() != -1 && numericRPG->firstVisitedProposition[(*it)->getStateID()] <= i){
				if (drand48() <= propositionSelectionRatio){

					parameter_symbol_list * pl = new VAL::parameter_symbol_list;
					for(VAL::parameter_symbol_list::iterator it2 = (*it)->toProposition()->args->begin();it2 != (*it)->toProposition()->args->end();++it2)
					{
						pl->push_back(*it2);
					}
					proposition *prop = new VAL::proposition((*it)->toProposition()->head,pl);
					shared_ptr <goal> simpleGoal (new simple_goal(prop, E_POS));
					milestones[i].push_back(simpleGoal);
				}
			}
		}
	}
}

SketchyPlan SketchyPlan::crossover(SketchyPlan *mother) {

}

SketchyPlan SketchyPlan::mutation() {

}

void SketchyPlan::print(){
	int length = milestones.size();
	cout << "Printing milestones with the length of: " << length << endl;
	for (int i = 0; i < length; i++){
		cout << "Layer: " << i << " ==> ";
		int mySize = milestones[i].size();
		for (int j = 0; j < mySize; j++){
			simple_goal *simpleGoal = dynamic_cast <simple_goal *> (milestones[i][j].get());
			if (simpleGoal){
				FastEnvironment env(0);
				Literal lit (simpleGoal->getProp(), &env);
				Literal *lit2 = instantiatedOp::getLiteral(&lit);
				lit2->write(cout);
			}else{
				milestones[i][j]->write(cout);
			}
			cout << ", ";
		}
		cout << endl;
	}
}


SketchyPlan::~SketchyPlan() {}

