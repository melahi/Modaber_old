

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

SketchyPlan::SketchyPlan(int length): length (length), fitness(-1) {
	createRandomSketchyPlan(length);
}

SketchyPlan::SketchyPlan(): length(0), fitness(-1){
}


void SketchyPlan::createRandomSketchyPlan(int length) {
	this->length = length;
	nStateVariables = myProblem.stateVariables.size();

	if (length < 2)
		return;


	stateValues = vector <vector <MyStateValue*> > (nStateVariables, vector <MyStateValue*> (length, NULL) );

	myProblem.environment.prepare(length);

	for (int i = 0; i < nStateVariables; i++){
		buildingWalk(i, length - 1);
	}
}

void SketchyPlan::buildingWalk(int variableId, int layerNumber) {
	if (layerNumber < 1)
		return;

	if (layerNumber == length - 1){
		if (myProblem.goalValue[variableId] != NULL){
			stateValues[variableId][layerNumber] = myProblem.goalValue[variableId];
		}else{
			int domainSize = myProblem.stateVariables[variableId].domain.size();
			vector <double> temp(domainSize, 0);
			double sum = 0;
			for (int j = 0; j < domainSize; j++){
				if (myProblem.stateVariables[variableId].domain[j].firstVisitedLayer <= layerNumber){
					sum += 1;
					temp[j] = 1;
				}
			}
			normolizing(temp, sum);
			int selectedStateValueIndex = selectRandomly(temp);
			stateValues[variableId][layerNumber] = &(myProblem.stateVariables[variableId].domain[selectedStateValueIndex]);
		}
		buildingWalk(variableId, layerNumber -1);
		return;
	}

	MyStateValue *nextStateValue = stateValues[variableId][layerNumber + 1];
	int j = selectRandomly(myProblem.environment.probability[layerNumber][variableId][nextStateValue->valueId]);
	stateValues[variableId][layerNumber] = &(myProblem.stateVariables[variableId].domain[j]);
	buildingWalk(variableId, layerNumber - 1);

}


void SketchyPlan::increaseOneLayer(){
	length++;
	if (length <= 2){
		createRandomSketchyPlan(length);
		return;
	}
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

SketchyPlan SketchyPlan::crossover(const SketchyPlan *mother) const{

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

SketchyPlan SketchyPlan::mutate() const{
	SketchyPlan child(*this);
	int layerNumber = drand48() * (length);
	int stateVariableId = drand48() * (nStateVariables);
	child.buildingWalk(stateVariableId, layerNumber);
	return child;
}

void SketchyPlan::write(ostream &sout){
	sout << "Printing milestones with the length of: " << length << endl;
	for (int j = 0; j < nStateVariables; ++j){
		sout << "{ ";
		for (int i = 1; i < length; i++){
			if (i != 1){
				sout << ", ";
			}
			sout << "[ " << j << ", " << i << " ==> " << stateValues[j][i]->valueId << "]";
		}
		sout << " }" << endl;
	}
	sout << "Fitness: " << fitness << endl;
}


SketchyPlan::~SketchyPlan() {}

