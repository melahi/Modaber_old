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
#include "Utilities.h"
#include "VALfiles/parsing/ptree.h"
#include "MyProblem.h"
#include <cvc4/cvc4.h>
#include <vector>
#include <cmath>
#include <sstream>

using namespace CVC4;
using namespace std;
using namespace Inst;
using namespace VAL;
using namespace mdbr;


class CVC4Problem {
public:


	void guaranteeSize (unsigned int maxSignificantTimePoint);


	void initialization();

	CVC4Problem (int nVariables, int nProposition, int nAction);

	//Start to build new clause for SMT problem
	virtual void startNewClause();

	virtual void endClause();

	void addLiteral ( polarity plrty, const proposition *prop, FastEnvironment *env, int significantTimePoint);

	//Add new boolean condition to the building clause
	virtual void addConditionToCluase(int propostion, int significantTimePoint, bool polarity);


	//Add new action to the building clause
	virtual void addActionToClause (int actionId, int significantTimePoint, bool polarity);

	//Add new numerical condition to the building clause
	virtual void AddConditionToCluase(const comparison* numericalCondition, FastEnvironment *env, int significantTimePoint);


	//Add new numerical assignment to the building clause
	virtual void AddConditionToCluase(const assignment* numericalAssignment, FastEnvironment *env, int significantTimePoint);

	void AddEqualityCondition (int variableId1, int significantTimePoint1, int variableId2, int significantTimePoint2);

	double solve(const Expr &assertExpr);

	void print();

	bool isActionUsed (int actionId, int significantTimePoint);

	void push();

	void pop();

	void insertAssertion (const Expr &e);

	Expr getAssertions ();

	Expr andAssertionList (unsigned int begin, unsigned int end);

	void clearAssertionList ();

	unsigned int getMaximumSignificantTimePoint() {return maximumSignificantTimePoint;}

	Expr simplify (const Expr & expr){ return smt.simplify(expr); }

	void assertFormula  () { smt.assertFormula (getAssertions());}
	void assertFormula  (const Expr &expr) { smt.assertFormula (expr);}


	/* I don't know how to implement following function, perhaps it's not bad to learn it!
	CVC4Problem& operator=(const CVC4Problem&);
	*/

	virtual ~CVC4Problem();

private:

	static ExprManager em;
	SmtEngine smt;


	static vector <Expr> variableExpr;
	static vector <Expr> propositionExpr;
	static vector <Expr> actionExpr;
	static unsigned int maximumSignificantTimePoint;


	vector <Expr> buildingClause;
	vector <Expr> assertions;
	int nVariables;
	int nProposition;
	int nAction;
	unsigned long int resourceLimit;
	unsigned long int lastNumberOfDecision;


	//find and return the index of corresponding PVariableExpression in the variableExpr array
	int getVariableIndex (int variableStateId, int significantTimePoint);

	//find and return the index of corresponding proposition in the propositionExpr array
	inline int getPropositionIndex (int proposition, int significantTimePoint);

	//find and return the index of corresponding action in the actionExpr array
	inline int getActionIndex (int action, int significantTimePoint);


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
					double myDouble = myProblem.initialValue[pne2->getGlobalID()];
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



#endif /* CVC4PROBLEM_H_ */
