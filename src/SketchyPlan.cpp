

#include "SketchyPlan.h"
#include "Utilities.h"
#include "MyProblem.h"
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
	this->length = length;
	nStateVariables = myProblem.stateVariables.size();

	if (length < 2)
		return;


	stateValues = vector <vector <MyStateValue*> > (nStateVariables, vector <MyStateValue*> (length, NULL) );

	myProblem.environment.prepare(length);

	createStateValuesForLastLayer();

	for (int i = 0; i < nStateVariables; i++){
		buildingWalk(i, length - 2);
	}
}

void SketchyPlan::buildingWalk(int variableId, int layerNumber) {
	if (layerNumber < 1)
		return;

	MyStateValue *nextStateValue = stateValues[variableId][layerNumber + 1];
	int j = selectRandomly(myProblem.environment.probability[layerNumber][variableId][nextStateValue->valueId]);
	stateValues[variableId][layerNumber] = &(myProblem.stateVariables[variableId].domain[j]);
	buildingWalk(variableId, layerNumber - 1);

}

void SketchyPlan::createStateValuesForLastLayer(){

	FastEnvironment env(0);
	createStateValuesForLastLayer(current_analysis->the_problem->the_goal, &env);

	for (int i = 0; i < nStateVariables; i++){
		if (stateValues[i][length - 1] == NULL){
			int domainSize = myProblem.stateVariables[i].domain.size();
			vector <double> temp(domainSize, 0);
			double sum = 0;
			for (int j = 0; j < domainSize; j++){
				if (myProblem.stateVariables[i].domain[j].firstVisitedLayer <= length - 1){
					sum += 1;
					temp[j] = 1;
				}
			}
			normolizing(temp, sum);
			int selectedStateValueIndex = selectRandomly(temp);
			stateValues[i][length - 1] = &(myProblem.stateVariables[i].domain[selectedStateValueIndex]);
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
		stateValues[stateValue->theStateVariable->variableId][length - 1] = stateValue;
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

void SketchyPlan::increaseOneLayer(){
	length++;
	fitness--;
	for (int i = 0; i < nStateVariables; i++){
		stateValues[i].push_back(stateValues[i][length - 2]);
	}
}

goal* SketchyPlan::convertPropositionToGoal(const proposition *originalProposition, polarity plrty){

	parameter_symbol_list * pl = new parameter_symbol_list;
	for(VAL::parameter_symbol_list::iterator it2 = originalProposition->args->begin();it2 != originalProposition->args->end(); ++it2)
	{
		pl->push_back(*it2);
	}
	proposition *prop = new proposition(originalProposition->head, pl);
	return new simple_goal(prop, plrty);
}

void SketchyPlan::convertStateValuesToMilestones(vector < vector < shared_ptr <goal> > > &milestones){
	milestones = vector <vector < shared_ptr <goal> > > (nStateVariables, vector < shared_ptr<goal> > (length) );

	for (int stateVariableId = 0; stateVariableId < nStateVariables; ++stateVariableId){
		for (int layerNumber = 1; layerNumber < length; ++layerNumber){

			if (stateValues[stateVariableId][layerNumber]->theProposition != NULL){
				goal *theGoal = convertPropositionToGoal(stateValues[stateVariableId][layerNumber]->theProposition->originalLiteral->toProposition(), E_POS);
				milestones[stateVariableId][layerNumber] = shared_ptr <goal> (theGoal);
			}else{
				MyStateVariable *theStateVariable = stateValues[stateVariableId][layerNumber]->theStateVariable;
				int theValueId = stateValues[stateVariableId][layerNumber]->valueId;
				int domainSize = theStateVariable->domain.size();
				goal_list *myGoalList = new goal_list();
				for (int i = 0; i < domainSize; i++){
					if (i != theValueId){
						goal * theGoal = convertPropositionToGoal(theStateVariable->domain[i].theProposition->originalLiteral->toProposition(), E_NEG);
						myGoalList->push_back(theGoal);
					}
				}
				milestones[stateVariableId][layerNumber] = shared_ptr <goal> (new conj_goal(myGoalList));
			}
		}
	}
}

SketchyPlan SketchyPlan::crossover(SketchyPlan *mother) {

	SketchyPlan child(*this);

	for (int i = 0; i < nStateVariables; i++){
		if (drand48() < 0.5){
			child.stateValues[i] = mother->stateValues[i];
		}/* else {
			child.stateValues[i] = this->stateValues[i];
		} */
	}

	return child;
}

SketchyPlan SketchyPlan::mutate() {
	SketchyPlan child(*this);
	int layerNumber = drand48() * (length - 1);
	int stateVariableId = drand48() * (nStateVariables);
	child.buildingWalk(stateVariableId, layerNumber);
	return child;
}

void SketchyPlan::print(){
/*
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
*/
}


SketchyPlan::~SketchyPlan() {}

