
#include "NumericRPG.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "CVC4Problem.h"
#include "Utilities.h"
#include <vector>
#include <iostream>
#include <cmath>

using namespace VAL;
using namespace Inst;
using namespace std;

void NumericRPG::constructFirstLayer() {

	//Literals
	pc_list<simple_effect*>::const_iterator it = current_analysis->the_problem->initial_state->add_effects.begin();
	pc_list<simple_effect*>::const_iterator itEnd = current_analysis->the_problem->initial_state->add_effects.end();
	FastEnvironment env(0);

	for (; it != itEnd; it++){
		Literal lit ((*it)->prop, &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() != -1){
			firstVisitedProposition[lit2->getStateID()] = 0;
		}
	}

	//PNEs
	CVC4Problem::updateInitialValues();


	pc_list<assignment*>::const_iterator asgnIt = current_analysis->the_problem->initial_state->assign_effects.begin();
	pc_list<assignment*>::const_iterator asgnItEnd = current_analysis->the_problem->initial_state->assign_effects.end();

	vector <double> firstLayerMinPNEValue, firstLayerMaxPNEValue;

	firstLayerMinPNEValue.resize(instantiatedOp::howManyNonStaticPNEs());
	firstLayerMaxPNEValue.resize(instantiatedOp::howManyNonStaticPNEs());

	for (; asgnIt != asgnItEnd; ++asgnIt){
		PNE pne ((*asgnIt)->getFTerm(), &env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2 && pne2->getStateID() != -1){
			firstLayerMaxPNEValue[pne2->getStateID()]= CVC4Problem::initialValue[pne2->getGlobalID()];
			firstLayerMinPNEValue[pne2->getStateID()]= CVC4Problem::initialValue[pne2->getGlobalID()];
		}
	}
	minPNEValue.push_back(firstLayerMinPNEValue);
	maxPNEValue.push_back(firstLayerMaxPNEValue);
}


inline double findMin (double t1, double t2, double t3, double t4){
	if (t2 < t1)
		t1 = t2;
	if (t3 < t1)
		t1 = t3;
	if (t4 < t1)
		t1 = t4;
	return t1;
}

inline double findMax (double t1, double t2, double t3, double t4){
	if (t2 > t1)
		t1 = t2;
	if (t3 > t1)
		t1 = t3;
	if (t4 > t1)
		t1 = t4;
	return t1;
}



void NumericRPG::estimateExpression (const expression *expr, FastEnvironment *env, double &minimumValue, double &maximumValue){

	//Binary expression
	const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		double leftMinValue, leftMaxValue, rightMinValue, rightMaxValue;
		estimateExpression(binary->getLHS(), env, leftMinValue, leftMaxValue);
		estimateExpression(binary->getRHS(), env, rightMinValue, rightMaxValue);
		if (dynamic_cast <const plus_expression* > (expr)){
			minimumValue = leftMinValue + rightMinValue;
			maximumValue = leftMaxValue + rightMaxValue;
		}else if (dynamic_cast<const minus_expression *> (expr)){
			minimumValue = leftMinValue - rightMaxValue;
			maximumValue = leftMaxValue - rightMinValue;
		}else if (dynamic_cast<const mul_expression *> (expr)) {
			double minMax, minMin, maxMax, maxMin;
			minMax = leftMinValue * rightMaxValue;
			minMin = leftMinValue * rightMinValue;
			maxMax = leftMaxValue * rightMaxValue;
			maxMin = leftMaxValue * rightMinValue;
			minimumValue = findMin(minMax, minMin, maxMax, maxMin);
			maximumValue = findMax(minMax, minMin, maxMax, maxMin);
		}else if (dynamic_cast<const div_expression *> (expr)){
			double minMax, minMin, maxMax, maxMin;
			minMax = leftMinValue / rightMaxValue;
			minMin = leftMinValue / rightMinValue;
			maxMax = leftMaxValue / rightMaxValue;
			maxMin = leftMaxValue / rightMinValue;
			minimumValue = findMin(minMax, minMin, maxMax, maxMin);
			maximumValue = findMax(minMax, minMin, maxMax, maxMin);
		}else{
			CANT_HANDLE("Can't evaluate min and max value of a binary_expression");
		}
		return;
	}

	//Unary Minus
	const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
	if (uMinus){
		double tempMax, tempMin;
		estimateExpression(uMinus->getExpr(), env, tempMin, tempMax);
		minimumValue = -1 * tempMax;
		maximumValue = -1 * tempMin;
		return;
	}

	//Constant
	const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
	if (numExpr){
		minimumValue = maximumValue = numExpr->double_value();
		return;
	}

	//Variable
	const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
	if (functionTerm){
		PNE pne = PNE(functionTerm, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2->getStateID() == -1){
			minimumValue = maximumValue = CVC4Problem::initialValue[pne2->getGlobalID()];
			return;
		}
		minimumValue = minPNEValue[numberOfLayers - 1][pne2->getStateID()];
		maximumValue = maxPNEValue[numberOfLayers - 1][pne2->getStateID()];
		return;
	}
	CANT_HANDLE("can't evaluate one expression for find min and max values");
	return;
}


bool NumericRPG::isPreconditionSatisfied(goal *precondition, FastEnvironment *env){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(precondition);
	if (simple){
		if (simple->getPolarity() == E_NEG){
			return true;
		}
		Literal lit(simple->getProp(), env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		if (firstVisitedProposition[lit2->getStateID()] != -1 && firstVisitedProposition[lit2->getStateID()] < numberOfLayers)
			return true;
		return false;
	}
	const comparison *comp = dynamic_cast<const comparison*> (precondition);
	if (comp){
		double leftMin, rightMin, leftMax, rightMax;
		estimateExpression(comp->getLHS(), env, leftMin, leftMax);
		estimateExpression(comp->getRHS(), env, rightMin, rightMax);
		bool lessThan, greaterThan, equality;
		lessThan = greaterThan = equality = false;
		if (EPSILON < (rightMax - leftMin)){
			lessThan = true;
		}
		if (EPSILON > (rightMin - leftMax)){
			greaterThan = true;
		}
		equality = !(lessThan ^ greaterThan);
		switch(comp->getOp()){
		case E_GREATER:
			return greaterThan;
		case E_GREATEQ:
			return (greaterThan | equality);
		case E_LESS:
			return lessThan;
		case E_LESSEQ:
			return (lessThan | equality);
		case E_EQUALS:
			return equality;
		}
		return true;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(precondition);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			if (!isPreconditionSatisfied(*it, env) ){
				return false;
			}
		}
		return true;
	}
	CANT_HANDLE("can't evaluate some precondition");
	return true;
}

bool NumericRPG::isApplicable (int actionId) {
	instantiatedOp *op = instantiatedOp::getInstOp(actionId);
	FastEnvironment *env = op->getEnv();
	return (isPreconditionSatisfied(op->forOp()->precondition, env));
}

void NumericRPG::applyAction (int actionId){

	firstVisitedAcotion[actionId] = numberOfLayers - 1;

	instantiatedOp *op = instantiatedOp::getInstOp(actionId);
	FastEnvironment *env = op->getEnv();
	addSimpleEffectList(op->forOp()->effects->add_effects, env);
	addAssignmentList(op->forOp()->effects->assign_effects, env);
}

void NumericRPG::addSimpleEffectList (const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env){
	pc_list<simple_effect*>::const_iterator it = simpleEffectList.begin();
	pc_list<simple_effect*>::const_iterator itEnd = simpleEffectList.end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		if (lit2->getStateID() != -1 && firstVisitedProposition[lit2->getStateID()] == -1){
			firstVisitedProposition[lit2->getStateID()] = numberOfLayers;
		}
	}
}

void NumericRPG::addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env){
	pc_list<assignment*>::const_iterator it = assignmentEffects.begin();
	pc_list<assignment*>::const_iterator itEnd = assignmentEffects.end();
	for (; it != itEnd; ++it){
		PNE pne ((*it)->getFTerm(),env);
		PNE *pne2 = instantiatedOp::getPNE(&pne);
		if (pne2->getStateID() != -1){
			double minimumValue, maximumValue;
			estimateExpression((*it)->getExpr(), env, minimumValue, maximumValue);
			minPNEValue[numberOfLayers][pne2->getStateID()] = fmin(minimumValue, minPNEValue[numberOfLayers][pne2->getStateID()]);
			maxPNEValue[numberOfLayers][pne2->getStateID()] = fmax(maximumValue, maxPNEValue[numberOfLayers][pne2->getStateID()]);
		}
	}
}


bool NumericRPG::extendOneLayer(){
	bool canContinue = false;

	//Inserting min value and max value vectors for the last layer
	vector <double> insertingLastLayerMinValue (minPNEValue[numberOfLayers - 1]);
	vector <double> insertingLastLayerMaxValue (maxPNEValue[numberOfLayers - 1]);
	minPNEValue.push_back(insertingLastLayerMinValue);
	maxPNEValue.push_back(insertingLastLayerMaxValue);



	//If there is any action which is not executed before, check its applicability, if it can be applied, then apply it!!!
	int nAction = firstVisitedAcotion.size();
	for (int i = 0; i < nAction; i++){
		if (firstVisitedAcotion[i] == -1){
			if (isApplicable(i)){
				applyAction(i);
				// If a new action can be applied it means that our graph doesn't level off and we should continue for another layer
				canContinue = true;
			}
		}
	}
	return canContinue;
}


NumericRPG::NumericRPG() {

	firstVisitedProposition.resize(instantiatedOp::howManyNonStaticLiterals(), -1);
	firstVisitedAcotion.resize(instantiatedOp::howMany(), -1);
	numberOfLayers = 0;
	constructFirstLayer();
	bool canContinue = true;
	while (canContinue){
		numberOfLayers++;
		/*
		 *@FIXME: we extend NumericRPG until no new action can be executed.
		 *		  So there can be a problem for numeric preconditions;
		 *		  If a numeric precondition can be satisfied after executing
		 *		  of a series of action we missed it!
		 */
		canContinue = extendOneLayer();
	}
	FastEnvironment env(0);
	minimumPlanLength = 1 + findMinimumLevelSatisfyingGoal(current_analysis->the_problem->the_goal, &env);
}

int NumericRPG::findMinimumLevelSatisfyingGoal(goal *gl, FastEnvironment *env){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
	if (simple){
		if (simple->getPolarity() == E_NEG){
			return 0;
		}
		Literal lit(simple->getProp(), env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		return firstVisitedProposition[lit2->getStateID()];
	}
	const comparison *comp = dynamic_cast<const comparison*> (gl);
	if (comp){
		for (int i = 0; i < numberOfLayers; i++){
			if (isPreconditionSatisfied(gl, env)){
				return i;
			}
		}
		return -1;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
	if (conjunctive){
		int ret = 0;
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			int temp = findMinimumLevelSatisfyingGoal(*it, env);
			if (temp == -1)
				return -1;
			if (temp > ret)
				ret = temp;
		}
		return ret;
	}
	CANT_HANDLE("can't evaluate some precondition");
	return 0;
}

void NumericRPG::print(ostream &sout){
	int nActions = firstVisitedAcotion.size();
	int nPropositions = firstVisitedProposition.size();
	int nPNEs = minPNEValue[0].size();
	vector <Literal*> nonStaticLiterals;
	nonStaticLiterals.resize(nPropositions);
	vector <PNE*> nonStaticPNEs;
	nonStaticPNEs.resize(nPNEs);

	for (LiteralStore::iterator it = instantiatedOp::literalsBegin(); it != instantiatedOp::literalsEnd(); ++it){
		if ((*it)->getStateID() != -1){
			nonStaticLiterals[(*it)->getStateID()] = *it;
		}
	}

	for (PNEStore::iterator it = instantiatedOp::pnesBegin(); it != instantiatedOp::pnesEnd(); ++it){
		if ((*it)->getStateID() != -1){
			nonStaticPNEs[(*it)->getStateID()] = *it;
		}
	}


	sout << "Number of Layers: " << numberOfLayers << endl;
	sout << "Lower bound for plan length: " << minimumPlanLength << endl;
	for (int i = 0; i < numberOfLayers; i++){
		for (int j = 0; j < nPropositions; j++){
			if (firstVisitedProposition[j] == i){
				nonStaticLiterals[j]->write(sout);
				sout << ';';
			}
		}
		sout << endl;

		for (int j = 0; j < nPNEs; j++){
			nonStaticPNEs[j]->write(sout);
			sout << " [" << minPNEValue[i][j] << ", " << maxPNEValue[i][j] << "];" ;
		}
		sout << endl;

		for (int j = 0; j < nActions; j++){
			if (firstVisitedAcotion[j] == i){
				instantiatedOp::getInstOp(j)->write(sout);
				sout << ';';
			}
		}
		sout << endl;
	}
}

NumericRPG::~NumericRPG() {
	// TODO Auto-generated destructor stub
}

