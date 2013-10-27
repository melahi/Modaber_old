/*
 * SolutionSimulator.cpp
 *
 *  Created on: Oct 22, 2013
 *      Author: sadra
 */

#include "SolutionSimulator.h"
#include "Utilities.h"

#include <limits>
#include <iostream>
using namespace std;


namespace mdbr {


double SolutionSimulator::evaluateExpression (const expression *expr, FastEnvironment *env, State &state){
	double ret = 0;
	//Binary expression
	const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
	if (binary){
		double left = evaluateExpression(binary->getLHS(), env, state);
		double right = evaluateExpression(binary->getRHS(), env, state);
		if (dynamic_cast <const plus_expression* > (expr)){
			ret = left + right;
		}else if (dynamic_cast<const minus_expression *> (expr)){
			ret = left - right;
		}else if (dynamic_cast<const mul_expression *> (expr)) {
			ret = left * right;
		}else if (dynamic_cast<const div_expression *> (expr)){
			ret = left / right;
		}else{
			CANT_HANDLE("binary_expression");
		}
		return ret;
	}

	//Unary Minus
	const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
	if (uMinus){
		ret = evaluateExpression(uMinus->getExpr(), env, state);
		return -1 * ret;
	}

	//Constant
	const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
	if (numExpr){
		ret = numExpr->double_value();
		return ret;
	}

	//Variable
	const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
	if (functionTerm){
		PNE pne = PNE(functionTerm, env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		ret = state.variableValues[pne2->getGlobalID()];
		return ret;
	}
	CANT_HANDLE("can't handle One expression in evaluating expression");
	return 0;

}



bool SolutionSimulator::isApplicable (goal *precondition, FastEnvironment *env, State &state){
	const simple_goal *simple = dynamic_cast<const simple_goal *>(precondition);

	if (simple){
		if (simple->getPolarity() == E_NEG){
			return true;
		}

		Literal lit(simple->getProp(), env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);


		if (state.trueLiterals.find(lit2->getGlobalID()) != state.trueLiterals.end()){
			return true;
		}
		return false;
	}

	const comparison *comp = dynamic_cast<const comparison*> (precondition);
	if (comp){

		double left = evaluateExpression(comp->getLHS(), env, state);
		double right = evaluateExpression(comp->getRHS(), env, state);
		switch(comp->getOp()){
		case E_GREATER:
			if (left - right > EPSILON){
				return true;
			}
			break;
		case E_GREATEQ:
			if (left - right >= -EPSILON){
				return true;
			}
			break;
		case E_LESS:
			if (left - right < -EPSILON){
				return true;
			}
			break;
		case E_LESSEQ:
			if (left - right <= EPSILON){
				return true;
			}
			break;
		case E_EQUALS:
			if (fabs(left - right) <= EPSILON){
				return true;
			}
			break;
		default:
			CANT_HANDLE ("We don't know the operator kind of numerical condition!!!");
			exit(0);
		}
		return false;
	}
	const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(precondition);
	if (conjunctive){
		const goal_list *goalList = conjunctive->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			if (!isApplicable(*it, env, state) ){
				return false;
			}
		}
		return true;
	}
	CANT_HANDLE("can't evaluate some precondition");
	return true;
}


void SolutionSimulator::apply (operator_ * op, FastEnvironment *env, State &state){
	applyAddEffect(&(op->effects->add_effects), env, state);
	applyDeleteEffect(&(op->effects->del_effects), env, state);
	applyAssignmentList(&(op->effects->assign_effects), env, state);
}

void SolutionSimulator::applyAddEffect (pc_list<simple_effect*> *addList, FastEnvironment *env, State &state){
	pc_list<simple_effect*>::const_iterator it = addList->begin();
	pc_list<simple_effect*>::const_iterator itEnd = addList->end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		state.trueLiterals.insert(lit2->getGlobalID());
	}
}

void SolutionSimulator::applyDeleteEffect (pc_list<simple_effect*> *deleteList, FastEnvironment *env, State &state){
	pc_list<simple_effect*>::const_iterator it = deleteList->begin();
	pc_list<simple_effect*>::const_iterator itEnd = deleteList->end();
	for (; it != itEnd; ++it){
		Literal lit ((*it)->prop,env);
		Literal *lit2 = instantiatedOp::getLiteral(&lit);
		state.trueLiterals.erase(lit2->getGlobalID());
	}
}


void SolutionSimulator::applyAssignmentList (const pc_list <assignment *> *assignmentEffects, FastEnvironment *env, State &state){
	pc_list<assignment*>::const_iterator it = assignmentEffects->begin();
	pc_list<assignment*>::const_iterator itEnd = assignmentEffects->end();

	for (; it != itEnd; ++it){

		PNE pne ( (*it)->getFTerm(), env );
		PNE *pne2 = instantiatedOp::getPNE(&pne);

		if (pne2->getStateID() != -1 && myProblem.variables[pne2->getStateID()].visitInPrecondition == false){
			continue;
		}

		double value = state.variableValues[pne2->getGlobalID()];

		double expressionValue = evaluateExpression((*it)->getExpr(), env, state);

		switch ((*it)->getOp()){
		case E_INCREASE:
			value += expressionValue;
			break;
		case E_DECREASE:
			value -= expressionValue;
			break;
		case E_SCALE_UP:
			value *= expressionValue;
			break;
		case E_SCALE_DOWN:
			value /= expressionValue;
			break;
		case E_ASSIGN:
			value = expressionValue;
			break;
		case E_ASSIGN_CTS:
			cerr << "Oops!!!, I don't know what is \"E_ASSIGN_CTS\"" << endl;
			exit(1);
			break;
		default:
			cerr << (*it)->getOp() << endl;
			cerr << "I think the program should never reach at this line, BTW we just was applying a numerical effect!" << endl;
			exit (1);
		}
		state.variableValues[pne2->getGlobalID()] = value;

		//The following line intended to add value to the domain of corresponding variable!!!
		if (pne2->getStateID() != -1){
			myProblem.variables[pne2->getStateID()].domain[value].value = value;
			myProblem.variables[pne2->getStateID()].domain[value].variable = &(myProblem.variables[pne2->getStateID()]);
		}
	}
}

int SolutionSimulator::countValues (){
	int nVariables = myProblem.variables.size();
	int ret = 0;
	for (int i = 0; i < nVariables; ++i){
		ret += myProblem.variables[i].domain.size();
		if (myProblem.variables[i].domain.find(undefinedValue) != myProblem.variables[i].domain.end()){
			ret --;
		}
	}
	return ret;
}

bool SolutionSimulator::isValid (vector <pair <operator_ *, FastEnvironment> >  &solution){
	State state;
	bool planIsValid = true;
	int nValueBeforeSimulation  = countValues();
	prepareInitialState(state);
	int planLength = solution.size();
	for (int i = 0; i < planLength; ++i){
		cout << "Operator (" << i << "): " << solution[i].first->name->getName() << "...   ";
		if (!isApplicable(solution[i].first->precondition, &(solution[i].second), state)){
			cout << "can't be applied!!!" << endl;
			planIsValid = false;
			break;
		}
		apply(solution[i].first, &(solution[i].second), state);
		cout << "Is applied!!!" << endl;
	}
	FastEnvironment env(0);
	if (planIsValid && !isApplicable(current_analysis->the_problem->the_goal, &env, state)){
		planIsValid = false;
	}
	int nValuesAfterSimulation = countValues();
	if (nValueBeforeSimulation != nValuesAfterSimulation){
		//Insert undefined values to each variable's domain
		int nVariables = myProblem.variables.size();
		for (int i = 0; i < nVariables; ++i){
			if (myProblem.variables[i].visitInPrecondition){
				myProblem.variables[i].domain[undefinedValue].value = undefinedValue;
				myProblem.variables[i].domain[undefinedValue].variable = &(myProblem.variables[i]);
			}
		}
	}else{
		//Delete undefined values from each variable's domain
		int nVariables = myProblem.variables.size();
		for (int i = 0; i < nVariables; ++i){
			myProblem.variables[i].domain.erase(undefinedValue);
		}
	}
	return planIsValid;
}


void SolutionSimulator::prepareInitialState(State &state){
	state.trueLiterals.clear();
	state.variableValues.resize(myProblem.initialValue.size());
	FastEnvironment env(0);
	applyAddEffect(&(current_analysis->the_problem->initial_state->add_effects), &env, state);
	applyAssignmentList(&(current_analysis->the_problem->initial_state->assign_effects), &env, state);
	//Inserting undefined value to domain of each variable
	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		if (myProblem.variables[i].visitInPrecondition == false){
			continue;
		}
		myProblem.variables[i].initialValue = &(myProblem.variables[i].domain[state.variableValues[myProblem.variables[i].originalPNE->getGlobalID()]]);
	}
}


SolutionSimulator::SolutionSimulator() {
}

SolutionSimulator::~SolutionSimulator() {
}

} /* namespace mdbr */
