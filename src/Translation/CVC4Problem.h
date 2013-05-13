/*
 * CVC4Problem.h
 *
 *  Created on: Feb 27, 2013
 *      Author: sadra
 */

#ifndef CVC4PROBLEM_H_
#define CVC4PROBLEM_H_

#include "SMTProblem.h"
#include <cvc4/cvc4.h>
#include <vector>
#include <cmath>


using namespace MyParser;
using namespace CVC4;




class CVC4Problem: public SMTProblem {
public:

	CVC4Problem (int nVariables, int nProposition, int nAction, int nSignificantTimePoint): em(), smt(&em), nVariables(nVariables), nProposition(nProposition), nAction(nAction), nSignificantTimePoint(nSignificantTimePoint) {

		smt.setOption("produce-models", SExpr("true"));
		smt.setOption("check-models", SExpr("true"));
		smt.setOption("produce-assignments", SExpr("true"));
		smt.setOption("verbosity", SExpr("0"));

		int size;
		//Creating variable expressions
		Type real = em.realType();
		size = nSignificantTimePoint * nVariables;
		variableExpr = new Expr [size];
		for (int i = 0; i < size; i++){
				variableExpr[i] = em.mkVar(real);

		}

		//Creating boolean expressions
		//For proposition
		Type boolean = em.booleanType();
		size = nSignificantTimePoint * nProposition;
		propositionExpr = new Expr [size];
		for (int i = 0; i < size; i++){
			propositionExpr[i] = em.mkVar(boolean);
		}

		//For action

//		@TODO: for now we doesn't target temporal problems, perhaps for future it will be not bad to handle temporal problems!
//		size = nSignificantTimePoint * nAction * 3;
//		actionExpr = new Expr[size]; // each part of start, overall and end of an action need an expression so for each action we should have 3 expressions

		size = nSignificantTimePoint * nAction;
		actionExpr = new Expr[size]; // each part of start, overall and end of an action need an expression so for each action we should have 3 expressions
		for (int i = 0; i < size; i++){
			actionExpr[i] = em.mkVar (boolean);
		}
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
	virtual void addActionToClause (int action, int significantTimePoint, bool polarity){
		int index = getActionIndex(action, significantTimePoint);
		if (polarity){
			buildingClause.push_back(actionExpr[index]);
		}else{
			buildingClause.push_back(em.mkExpr(kind::NOT, actionExpr[index]));
		}
	}

	//Add new numerical condition to the building clause
	virtual void AddConditionToCluase(PComparisonExpression* numericalCondition, int significantTimePoint){
		Expr left = convertPGroundExpressionToCVC4Expr(numericalCondition->getLHS(), significantTimePoint);
		Expr right = convertPGroundExpressionToCVC4Expr(numericalCondition->getRHS(), significantTimePoint);
		Kind operatorKind;
		switch(numericalCondition->op){
		case MyParser::E_GREATER:
			operatorKind = kind::GT;
			break;
		case MyParser::E_GREATEQ:
			operatorKind = kind::GEQ;
			break;
		case MyParser::E_LESS:
			operatorKind = kind::LT;
			break;
		case MyParser::E_LESSEQ:
			operatorKind = kind::LEQ;
			break;
		case MyParser::E_EQUALS:
			operatorKind = kind::EQUAL;
			break;
		default:
			cerr << "We don't know the operator of numerical condition!!!" << endl;
			exit(0);
		}
		buildingClause.push_back(em.mkExpr (operatorKind, left, right));
	}

	//Add new numerical assignment to the building clause
	virtual void AddConditionToCluase(PAssignment* numericalAssignment, int significantTimePoint){
		int variableIndex = getVariableIndex(numericalAssignment->var, significantTimePoint);
		Expr result = convertPGroundExpressionToCVC4Expr(numericalAssignment,significantTimePoint);
		switch (numericalAssignment->op){
		case MyParser::E_INCREASE:
			result = em.mkExpr(kind::PLUS, variableExpr[variableIndex], result);
			break;
		case MyParser::E_DECREASE:
			result = em.mkExpr(kind::MINUS, variableExpr[variableIndex], result);
			break;
		case MyParser::E_SCALE_UP:
			result = em.mkExpr(kind::MULT, variableExpr[variableIndex], result);
			break;
		case MyParser::E_SCALE_DOWN:
			result = em.mkExpr(kind::DIVISION, variableExpr[variableIndex], result);
			break;
		case MyParser::E_ASSIGN_CTS:
			cerr << "Oops!!!, I don't know what is \"E_ASSIGN_CTS\"" << endl;
			exit(1);
			break;
		default:
			cerr << "I think the program should never reach at this line, BTW we just was processing a numerical assignment!" << endl;
			exit (1);
		}

		buildingClause.push_back(em.mkExpr(kind::EQUAL, variableExpr[variableIndex], result));
	}


	void solve(){

		// TODO: For now, we don't considered processing time, we should consider it ASAP
		// TODO: We need statistical information

		cout << "Start to try solving the problem" << endl;
		Result result = smt.checkSat();

		switch (result.isSat()){
		case Result::SAT:
			cout << "OH yeay!, the problem is solved" << endl;
			break;
		case Result::UNSAT:
			cout << "The problem is not satisfiable!!!" << endl;
			break;
		default:
			cout << "The result is neither \"SAT\" nor \"UNSAT\"!!" << endl;
			break;
		}
	}



	~CVC4Problem(){
		delete(variableExpr);
		delete(propositionExpr);
		delete(actionExpr);
	}


private:

	ExprManager em;
	SmtEngine smt;

	Expr *variableExpr;
	Expr *propositionExpr;
	Expr *actionExpr;
	vector <Expr> buildingClause;
	int nVariables;
	int nProposition;
	int nAction;
	int nSignificantTimePoint;

	//find and return the index of corresponding PVariableExpression in the variableExpr array
	inline int getVariableIndex (const PVariableExpression * variable, int significantTimePoint){
		return significantTimePoint * nVariables + variable->variableID;
	}

	//find and return the index of corresponding proposition in the propositionExpr array
	inline int getPropositionIndex (int proposition, int significantTimePoint){
		return significantTimePoint * nProposition + proposition;
	}

	//find and return the index of corresponding action in the actionExpr array
	inline int getActionIndex (int action, int significantTimePoint){
		return significantTimePoint * nVariables + action;
	}

	Expr convertPGroundExpressionToCVC4Expr (const PGroundExpression* pGroundExpr, int significantTime){

		//Constant
		const PConstantExpression *constantExpr = dynamic_cast < const PConstantExpression* > (pGroundExpr);
		if (constantExpr){
			int nominator, denominator;
			simpleConvertToRational(constantExpr->value, nominator, denominator);
			return em.mkConst(Rational(nominator, denominator));
		}

		//Variable
		const PVariableExpression *pVariableExpr = dynamic_cast <const PVariableExpression* > (pGroundExpr);
		if (variableExpr){
			int index = getVariableIndex(pVariableExpr, significantTime);
			return variableExpr[index];
		}

		//UMinus
		const PUMinusExpression* pUMinusExpr = dynamic_cast <const PUMinusExpression* > (pGroundExpr);
		if (pUMinusExpr){
			Expr uMinusExpr = convertPGroundExpressionToCVC4Expr(pUMinusExpr->getOperand(), significantTime);
			return em.mkExpr(kind::UMINUS, uMinusExpr);
		}


		//Operators
		const POperationExpression* pOperationExpr = dynamic_cast<const POperationExpression*> (pGroundExpr);
		if (pOperationExpr){
			Expr left = convertPGroundExpressionToCVC4Expr(pOperationExpr->getLHS(), significantTime);
			Expr right = convertPGroundExpressionToCVC4Expr(pOperationExpr->getRHS(), significantTime);

			switch (pOperationExpr->getOpt()){
			case E_PLUS:
				return em.mkExpr(kind::PLUS, left, right);
			case E_MINUS:
				return em.mkExpr(kind::MINUS, left, right);
			case E_MUL:
				return em.mkExpr(kind::MULT, left, right);
			case E_DIV:
				return em.mkExpr(kind::DIVISION, left, right);
			}
		}

		//I don't know??
		cerr << "Oops, something wrong, I don't know what is the type of PGroundExpression in the \"convertPGroundExpressionToCVC4Expr\" function !!!"<< endl;
		exit(1);
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


#endif /* CVC4PROBLEM_H_ */
