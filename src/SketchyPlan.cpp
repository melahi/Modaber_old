

#include "SketchyPlan.h"
#include "Utilities.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/parsing/ptree.h"
#include <vector>
#include <set>



using namespace std;
using namespace VAL;
using namespace Inst;

SketchyPlan::SketchyPlan(int length): length (length) {
	fitness = -1;
	createRandomSketchyPlan(length);
}


void SketchyPlan::createRandomSketchyPlan(int length) {
	if (length < 3)
		return;

	this->length = length;
	nStateVariables = myProblem.stateVariables.size();

	stateValues = vector <vector <MyStateValue*> > (nStateVariables, vector <MyStateValue*> (length, NULL) );

	createStateValuesForLastLayer();

	for (int i = 0; i < nStateVariables; i++){
		buildingWalk(i);
	}
}

void SketchyPlan::buildingWalk(int variableId) {
	myProblem.environment.prepare(length);

	for (int layerNumber = length - 2; layerNumber >= 0; layerNumber--){
		MyStateValue *nextStateValue = stateValues[variableId][layerNumber + 1];
		int j = selectRandomly(myProblem.environment.probability[layerNumber][variableId][nextStateValue->valueId]);
		stateValues[variableId][layerNumber] = &(myProblem.stateVariables[variableId].domain[j]);
	}
}

void SketchyPlan::createStateValuesForLastLayer(){

	FastEnvironment env(0);
	createStateValuesForLastLayer(current_analysis->the_problem->the_goal, &env);

	for (int i = 0; i < nStateVariables; i++){
		if (stateValues[length - 1][i] == NULL){
			int domainSize = myProblem.stateVariables[i].domain.size();
			vector <double> temp(domainSize, 0);
			double sum = 0;
			for (int j = 0; j < domainSize; j++){
				if (myProblem.stateVariables[i].domain[j].theProposition->firstVisitedLayer <= length - 1){
					sum += 1;
					temp[j] = 1;
				}
			}
			normolizing(temp, sum);
			int selectedStateValueIndex = selectRandomly(temp);
			stateValues[length - 1][i] = myProblem.stateVariables[i].domain[selectedStateValueIndex];
		}
	}
}

void SketchyPlan::createStateValuesForLastLayer(goal *the_goal, FastEnvironment *env){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(the_goal);
	if (simple){

		Literal lit (simple->getProp(), env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);

		if (!lit2){
			CANT_HANDLE("Warning: can't find some literal in creating state values!!!");
			lit2->write(cerr);
			return;
		}

		if (lit2->getStateID() == -1){
			return;
		}

		MyStateValue *stateValue = myProblem.propositions[lit2->getStateID()].stateValue;
		stateValues[length - 1][stateValue->theStateVariable->variableId] = stateValue;
		return;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(the_goal);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			createStateValuesForLastLayer(*it, env);
		}
		return;
	}
	CANT_HANDLE("Can't create some state value from some goals!!!");
}


void SketchyPlan::convertStateValuesToMilestones(){
	milestones = vector <vector < shared_ptr <goal> > > (length, vector < shared_ptr<goal> > (nStateVariables, shared_ptr<goal> (NULL)) );

	for (int layerNumber = 0; layerNumber < length; ++layerNumber){
		for (int stateValueId = 0; stateValueId < nStateVariables; ++stateValueId){

			//Creating a goal from selected proposition
			const proposition *originalProposition = stateValues[layerNumber][stateValueId]->theProposition->originalLiteral->toProposition();
			parameter_symbol_list * pl = new parameter_symbol_list;
			for(VAL::parameter_symbol_list::iterator it2 = originalProposition->args->begin();it2 != originalProposition->args->end(); ++it2)
			{
				pl->push_back(*it2);
			}
			proposition *prop = new proposition(originalProposition->head,pl);

			milestones[layerNumber][stateValueId] = shared_ptr <goal> (new simple_goal(prop, E_POS));
		}
	}
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

