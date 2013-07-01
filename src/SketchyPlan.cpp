

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

SketchyPlan::SketchyPlan(NumericRPG *numericRPG, int length): numericRPG(numericRPG), length (length) {
	propositionSelectionRatio = 1.0 / 50.0;
	fitness = -1;
	createRandomSketchyPlan(length);
}

void SketchyPlan::createRandomIntermediateGoalLayer(int layerNumber){

	milestones[layerNumber].clear();

	LiteralStore::iterator it, itEnd;
	itEnd = instantiatedOp::literalsEnd();
	it = instantiatedOp::literalsBegin();


	for (;it != itEnd; it++){
		if ((*it)->getStateID() != -1 && numericRPG->firstVisitedProposition[(*it)->getStateID()] <= layerNumber){
			if (drand48() <= propositionSelectionRatio){

				//Creating a goal from selected proposition
				parameter_symbol_list * pl = new VAL::parameter_symbol_list;
				for(VAL::parameter_symbol_list::iterator it2 = (*it)->toProposition()->args->begin();it2 != (*it)->toProposition()->args->end();++it2)
				{
					pl->push_back(*it2);
				}
				proposition *prop = new VAL::proposition((*it)->toProposition()->head,pl);
				shared_ptr <goal> simpleGoal (new simple_goal(prop, E_POS));


				milestones[layerNumber].push_back(simpleGoal);
			}
		}
	}


}

void SketchyPlan::createRandomSketchyPlan(int length) {
	if (length < 3)
		return;
	milestones.resize(length);


	for (int i = 1; i < length - 1; i += 2){
		createRandomIntermediateGoalLayer(i);
	}
}

void SketchyPlan::increaseOneLayer(){
	length++;
	milestones.resize(length);

	//Perhaps its not bad to do something after resizing the milestones!!!

}

SketchyPlan SketchyPlan::crossover(SketchyPlan *mother) {
	int firstEffectiveLayerOfFather, lastEffectiveLayerOfMother;

	firstEffectiveLayerOfFather = length + 1;
	lastEffectiveLayerOfMother = -1;

	for (int i = 0; i < length; i++){
		if (i < firstEffectiveLayerOfFather && milestones[i].size()){
			firstEffectiveLayerOfFather = i;
		}
		if (mother->milestones[i].size()){
			lastEffectiveLayerOfMother = i;
		}
	}

	if ( firstEffectiveLayerOfFather >= lastEffectiveLayerOfMother){
		CANT_HANDLE("WARNING: in crossover the child is exactly looks like as its father");
		return *this;
	}



	int randomSplittingPoint = (lastEffectiveLayerOfMother - firstEffectiveLayerOfFather) * drand48() + firstEffectiveLayerOfFather + 1;
	SketchyPlan child(*this);
	for (int i = randomSplittingPoint; i < length; i++){
		child.milestones[i] = mother->milestones[i];
	}
	return child;
}

SketchyPlan SketchyPlan::mutate() {
	SketchyPlan child(*this);
	vector <int> effectiveLayer;

	/* The first method of finding effective layers
	for (int i = 0; i < length; i++){
		if (child.milestones[i].size()){
			effectiveLayer.push_back(i);
		}
	}
	*/

	// The second method of considering effective layers
	for (int i = 1; i < length - 1; i += 2){
		effectiveLayer.push_back(i);
	}

	int selectedLayer = (drand48() * effectiveLayer.size());
	child.createRandomIntermediateGoalLayer(effectiveLayer[selectedLayer]);
	return child;
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

