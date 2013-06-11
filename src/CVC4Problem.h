/*
 * CVC4Problem.h
 *
 *  Created on: Feb 27, 2013
 *      Author: sadra
 */

#ifndef CVC4PROBLEM_H_
#define CVC4PROBLEM_H_

#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include <cvc4/cvc4.h>
#include <vector>
#include <cmath>
#include <sstream>

using namespace CVC4;
using namespace std;


#define CANT_HANDLE(x) cerr << "********************************" << x << "********************************" << endl;




class CVC4Problem {
public:

	static vector <double> initialValue;		//This vector contains initial value for each function (variable); Index of this vector corresponds to global variable of the function (variable);

	static void updateInitialValues (){
		if (CVC4Problem::initialValue.size() > 0){
			//initialValue vector has been initialized so we can return!
			return;
		}

		pc_list<assignment*>::const_iterator it = current_analysis->the_problem->initial_state->assign_effects.begin();
		pc_list<assignment*>::const_iterator itEnd = current_analysis->the_problem->initial_state->assign_effects.end();
		CVC4Problem::initialValue.resize(current_analysis->the_problem->initial_state->assign_effects.size());    //we assume in the initial state the value of every function (variable) has been declared
		FastEnvironment env(0);

		for (; it != itEnd; ++it){
			PNE pne ((*it)->getFTerm(), &env);
			PNE *pne2 = instantiatedOp::findPNE(&pne);
			const num_expression *numExpr = dynamic_cast <const num_expression *>((*it)->getExpr());
			if (pne2 && numExpr && (*it)->getOp() == E_ASSIGN){
				CVC4Problem::initialValue[pne2->getGlobalID()] = numExpr->double_value();
			}else{
				CANT_HANDLE("Can't find Some Initial Value ")
			}
		}
		return;
	}


	void increaseSizeForAnotherStep (bool initialStep = false){

		int size;

		//Creating variable expressions
		Type real = em.realType();
		size = variableExpr.size();
		for (int i = 0; i < nVariables; i++){
			ostringstream oss;
			oss << "variable(" << (size + i) % nVariables << "," << (size + i) / nVariables << ")";
			variableExpr.push_back(em.mkVar(oss.str(),real));
		}


		//Creating boolean expressions


		//For proposition
		Type boolean = em.booleanType();
		size = propositionExpr.size();
		for (int i = 0; i < nProposition; i++){
			ostringstream oss;
			oss << "proposition(" << (size + i) % nProposition << "," << (size + i) / nProposition << ")";
			propositionExpr.push_back(em.mkVar(oss.str(), boolean));
		}


		//For action

		if (!initialStep){

			//		@TODO: for now we doesn't target temporal problems, perhaps for future it will be not bad to handle temporal problems!
			//		size = nSignificantTimePoint * nAction * 3;

			size = actionExpr.size();
			for (int i = 0; i < nAction; i++){
				ostringstream oss;
				oss << "action(" << (size + i) % nAction << "," << (size + i) / nAction << ")";
				actionExpr.push_back(em.mkVar (oss.str(), boolean));
			}

		}

	}

	CVC4Problem (int nVariables, int nProposition, int nAction): em(), smt(&em), nVariables(nVariables), nProposition(nProposition), nAction(nAction) {

		CVC4Problem::updateInitialValues();

		smt.setOption("produce-models", SExpr("true"));
		smt.setOption("check-models", SExpr("false"));		//In the case of debugging we can turn it to "true"
		smt.setOption("interactive-mode", SExpr("false"));		//In the case of debugging we can turn it to "true"
		smt.setOption("produce-assignments", SExpr("true"));
		smt.setOption("verbosity", SExpr("0"));
		smt.setOption("incremental", SExpr("true"));

		increaseSizeForAnotherStep(true);
	}

	//Start to build new clause for SMT problem
	virtual void startNewClause(){
		buildingClause.clear();
	}

	//By calling this function, you mean the clause is already built and it should be inserted to the SMT problem
	virtual void endClause(){
		if (buildingClause.size() == 0)
			return;
		if (buildingClause.size() == 1){
			smt.assertFormula(buildingClause[0]);
			return;
		}
		smt.assertFormula(em.mkExpr(kind::OR, buildingClause));
		return;
	}

	inline void addLiteral ( polarity plrty, const proposition *prop, FastEnvironment *env, int significantTimePoint){
		Literal lit (prop, env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() == -1)
			return;
		addConditionToCluase(lit2->getStateID(), significantTimePoint, (plrty == E_POS));
	}


	//Add new boolean condition to the building clause
	virtual void addConditionToCluase(int propostion, int significantTimePoint, bool polarity){
		int index = getPropositionIndex(propostion, significantTimePoint);
		if (polarity){
			buildingClause.push_back(propositionExpr[index]);
		}else{
			buildingClause.push_back(em.mkExpr(kind::NOT, propositionExpr[index]));
		}
	}

	//Add new action to the building clause
	virtual void addActionToClause (int actionId, int significantTimePoint, bool polarity){
		int index = getActionIndex(actionId, significantTimePoint);
		if (polarity){
			buildingClause.push_back(actionExpr[index]);
		}else{
			buildingClause.push_back(em.mkExpr(kind::NOT, actionExpr[index]));
		}
	}

	//Add new numerical condition to the building clause
	virtual void AddConditionToCluase(const comparison* numericalCondition, FastEnvironment *env, int significantTimePoint){
		ExpressionConvertor myConvertor(env, this, significantTimePoint);
		Expr left = myConvertor.convertExpressionToCVC4Expr(numericalCondition->getLHS());
		Expr right = myConvertor.convertExpressionToCVC4Expr(numericalCondition->getRHS());
		Kind operatorKind;
		switch(numericalCondition->getOp()){
		case E_GREATER:
			operatorKind = kind::GT;
			break;
		case E_GREATEQ:
			operatorKind = kind::GEQ;
			break;
		case E_LESS:
			operatorKind = kind::LT;
			break;
		case E_LESSEQ:
			operatorKind = kind::LEQ;
			break;
		case E_EQUALS:
			operatorKind = kind::EQUAL;
			break;
		default:
			CANT_HANDLE ("We don't know the operator kind of numerical condition!!!");
			exit(0);
		}
		buildingClause.push_back(em.mkExpr (operatorKind, left, right));
	}

	//Add new numerical assignment to the building clause
	virtual void AddConditionToCluase(const assignment* numericalAssignment, FastEnvironment *env, int significantTimePoint){
		ExpressionConvertor variableConvertor(env, this, significantTimePoint);
		Expr variable = variableConvertor.convertExpressionToCVC4Expr(numericalAssignment->getFTerm());
		if (variable.isConst()){
			//This case is happening just in initial case when initial state determine the value of a static variable
			return;
		}
		ExpressionConvertor expressionConvertor(env, this, significantTimePoint - 1);
		Expr result = expressionConvertor.convertExpressionToCVC4Expr(numericalAssignment->getExpr());

		Kind assignmentOperator = kind::EQUAL;
		switch (numericalAssignment->getOp()){
		case E_INCREASE:
			assignmentOperator = kind::PLUS;
			break;
		case E_DECREASE:
			assignmentOperator = kind::MINUS;
			break;
		case E_SCALE_UP:
			assignmentOperator = kind::MULT;
			break;
		case E_SCALE_DOWN:
			assignmentOperator = kind::DIVISION;
			break;
		case E_ASSIGN:
			break;
		case E_ASSIGN_CTS:
			cerr << "Oops!!!, I don't know what is \"E_ASSIGN_CTS\"" << endl;
			exit(1);
			break;
		default:
			cerr << numericalAssignment->getOp() << endl;
			cerr << "I think the program should never reach at this line, BTW we just was processing a numerical assignment!" << endl;
			exit (1);
		}

		if (assignmentOperator != kind::EQUAL){
			Expr variableInPreviousTime = expressionConvertor.convertExpressionToCVC4Expr(numericalAssignment->getFTerm());
			result = em.mkExpr(assignmentOperator, variableInPreviousTime, result);
		}

		buildingClause.push_back(em.mkExpr(kind::EQUAL, variable, result));
	}

	void AddEqualityCondition (int variableId1, int significantTimePoint1, int variableId2, int significantTimePoint2){
		int variableIndex1 = getVariableIndex(variableId1, significantTimePoint1);
		int variableIndex2 = getVariableIndex(variableId2, significantTimePoint2);
		Expr equalityCondition = em.mkExpr(kind::EQUAL, variableExpr[variableIndex1], variableExpr[variableIndex2]);
		buildingClause.push_back(equalityCondition);
	}


	bool solve(){

		// TODO: For now, we don't considered processing time, we should consider it ASAP
		// TODO: We need statistical information

		cout << "Start to try solving the problem" << endl;
		Result result = smt.checkSat();



		/*      For debug
		Statistics myStatistics = smt.getStatistics();
		cout << "*****************************Start****************************" << endl;
		myStatistics.flushInformation(cout);
		cout << "******************************End*****************************" << endl;
		*/


		switch (result.isSat()){
		case Result::SAT:
			cout << "OH yeay!, the problem is solved" << endl;
			return true;
			break;
		case Result::UNSAT:
			cout << "The problem is not satisfiable!!!" << endl;
			break;
		default:
			cout << "The result is neither \"SAT\" nor \"UNSAT\"!!" << endl;
			break;
		}
		return false;
	}

	void print(){
		vector <Expr> assertions = smt.getAssertions();
		for (size_t i = 0; i < assertions.size(); i++){
			if (i){
				cout << "AND   ";
			}
			assertions[i].toStream(cout);
			cout << endl;
		}
	}

	bool isActionUsed (int actionId, int significantTimePoint){
		int actionIndex = getActionIndex(actionId, significantTimePoint);
		bool isUsed = smt.getValue(actionExpr[actionIndex]).getConst<bool>();
		return isUsed;
	}

	void push(){
		smt.push();
	}
	void pop(){
		smt.pop();
	}


	virtual ~CVC4Problem(){
	}


private:

	ExprManager em;
	SmtEngine smt;

	vector <Expr> variableExpr;
	vector <Expr> propositionExpr;
	vector <Expr> actionExpr;


	vector <Expr> buildingClause;
	int nVariables;
	int nProposition;
	int nAction;

	//find and return the index of corresponding PVariableExpression in the variableExpr array
	inline int getVariableIndex (int variableStateId, int significantTimePoint){
		return significantTimePoint * nVariables + variableStateId;
	}

	//find and return the index of corresponding proposition in the propositionExpr array
	inline int getPropositionIndex (int proposition, int significantTimePoint){
		return significantTimePoint * nProposition + proposition;
	}

	//find and return the index of corresponding action in the actionExpr array
	inline int getActionIndex (int action, int significantTimePoint){
		return significantTimePoint * nAction + action;
	}
	class ExpressionConvertor {
	public:
		FastEnvironment *env;
		CVC4Problem *cvc4Problem;
		int significantTimepoint;
		ExpressionConvertor (FastEnvironment *env, CVC4Problem *cvc4Problem, int significantTime): env(env), cvc4Problem(cvc4Problem), significantTimepoint(significantTime){};

		Expr convertExpressionToCVC4Expr (const expression* expr){

			//Binary expression
			const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
			if (binary){
				Kind operatorKind;
				if (dynamic_cast <const plus_expression* > (expr)){
					operatorKind = kind::PLUS;
				}else if (dynamic_cast<const minus_expression *> (expr)){
					operatorKind = kind::MINUS;
				}else if (dynamic_cast<const mul_expression *> (expr)) {
					operatorKind = kind::MULT;
				}else if (dynamic_cast<const div_expression *> (expr)){
					operatorKind = kind::DIVISION;
				}else{
					CANT_HANDLE("binary_expression");
				}
				Expr left = convertExpressionToCVC4Expr(binary->getLHS());
				Expr right = convertExpressionToCVC4Expr(binary->getRHS());
				return cvc4Problem->em.mkExpr(operatorKind, left, right);
			}

			//Unary Minus
			const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
			if (uMinus){
				Expr uMinusExpr = convertExpressionToCVC4Expr(uMinus->getExpr());
				return cvc4Problem->em.mkExpr(kind::UMINUS, uMinusExpr);
			}

			//Constant
			const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
			if (numExpr){
				long double myDouble = numExpr->double_value();
				int nominator, denominator;
				simpleConvertToRational(myDouble, nominator, denominator);
				return cvc4Problem->em.mkConst(Rational(nominator, denominator));
			}

			//Variable
			const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
			if (functionTerm){
				PNE pne = PNE(functionTerm, env);
				PNE *pne2 = instantiatedOp::findPNE(&pne);
				if (pne2->getStateID() == -1){
					double myDouble = CVC4Problem::initialValue[pne2->getGlobalID()];
					int nominator, denominator;
					simpleConvertToRational(myDouble, nominator, denominator);
					return cvc4Problem->em.mkConst(Rational(nominator, denominator));
				}
				int index = cvc4Problem->getVariableIndex(pne2->getStateID(), significantTimepoint);
				return cvc4Problem->variableExpr[index];
			}
			CANT_HANDLE("can't handle One expression in converting to CVC4EXPR");
			return Expr();
		}

		void simpleConvertToRational (double input, int &nominator, int &denominator){
			nominator = (int) input;
			denominator = 1;
			double inputFloor = (double) nominator;
			double epsilon = 1e-9;
			int MAX_INT = (1 << 30) / 10;
			while (fabs(input - inputFloor) > epsilon && abs(nominator) < MAX_INT && abs(denominator) < MAX_INT ){
				denominator *= 10;
				input *= 10;
				nominator = (int) input;
				inputFloor = (double) nominator;
			}
			return;
		}

	};


};

vector <double> CVC4Problem::initialValue;


#endif /* CVC4PROBLEM_H_ */
