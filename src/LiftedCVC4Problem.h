/*
 * CVC4Problem.h
 *
 *  Created on: Feb 27, 2013
 *      Author: sadra
 */

#ifndef LIFTEDCVC4PROBLEM_H_
#define LIFTEDCVC4PROBLEM_H_

#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "Utilities.h"
#include "VALfiles/parsing/ptree.h"
#include "MyProblem.h"
#include "MyAtom.h"
#include <cvc4/cvc4.h>
#include <vector>
#include <cmath>
#include <sstream>

using namespace CVC4;
using namespace std;
using namespace Inst;
using namespace VAL;
using namespace mdbr;


class LiftedCVC4Problem {
public:

	void guaranteeSize (unsigned int maxSignificantTimePoint);


	void initialization();

	LiftedCVC4Problem (int nVariables, int nProposition, int nPartialAction, int nUnificationId);

	//Start to build new clause for SMT problem
	virtual void startNewClause();

	virtual void endClause();

	void addLiteral ( polarity plrty, const proposition *prop, FastEnvironment *env, int operatorId, int significantTimePoint);

	void addPartialActionToClause (MyPartialAction *partialAction, int significantTimePoint, bool polarity);

	//Add new boolean condition to the building clause
	void addConditionToCluase(int propostion, int operatorId, int significantTimePoint, bool polarity);

	void AddConditionToCluase(const MyProposition *myProposition, int operatorId, int significantTimePoint, bool polarity);

	//Add new numerical condition to the building clause
	void AddConditionToCluase(const comparison* numericalCondition, FastEnvironment *env, int operatorId, int significantTimePoint);

	//Add new numerical assignment to the building clause
	void AddConditionToCluase(const assignment* numericalAssignment, FastEnvironment *env, int operatorId, int significantTimePoint);

	void AddEqualityCondition (int variableId1, int operatorId1, int significantTimePoint1, int variableId2, int operatorId2, int significantTimePoint2, bool polarity);

	void AddEqualityCondition (int variableId1, int operatorId, int significantTimePoint1, double value, bool polarity);

	bool solve(const Expr &assertExpr);

	void print();

	void print(vector <Expr> &expression);

//	void push();
//
//	void pop();

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

	void activePermanentChange() { permanentChange = true;}
	void inActivePermanentChange() { permanentChange = false;}
	virtual ~LiftedCVC4Problem();

private:

	ExprManager em;
	SmtEngine smt;

	int nVariables;
	int nPropositions;
	int nPartialActions;
	int nUnifications;


	vector <Expr> variableExpr;
	vector <Expr> propositionExpr;
	vector <Expr> unificationExpr;
	vector <Expr> partialActionExpr;

	Expr trueExpr;
	Expr falseExpr;

	unsigned int maximumSignificantTimePoint;


	vector <Expr> buildingClause;
	vector <Expr> assertions;


	bool ignoreCluase;
	bool permanentChange;


	//find and return the index of corresponding PVariableExpression in the variableExpr array
	int getVariableIndex (int variableId, int operatorId, int significantTimePoint);

	//find and return the index of corresponding proposition in the propositionExpr array
	int getPropositionIndex (int propositionId, int operatorId, int significantTimePoint);

	//find and return the index of corresponding action in the actionExpr array
	int getPartialActionIndex (MyPartialAction *partialAction, int significantTimePoint);

	//find and return the index of corresponding action in the actionExpr array
	int getUnificationIndex (int unificationId, int significantTimePoint);


	class ExpressionConvertor {
	public:
		FastEnvironment *env;
		LiftedCVC4Problem *liftedCVC4Problem;
		int operatorId;
		int significantTimepoint;
		ExpressionConvertor (FastEnvironment *env, CVC4Problem *cvc4Problem, int operatorId, int significantTime): env(env), liftedCVC4Problem(cvc4Problem), operatorId(operatorId), significantTimepoint(significantTime){};

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
				return liftedCVC4Problem->em.mkExpr(operatorKind, left, right);
			}

			//Unary Minus
			const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
			if (uMinus){
				Expr uMinusExpr = convertExpressionToCVC4Expr(uMinus->getExpr());
				return liftedCVC4Problem->em.mkExpr(kind::UMINUS, uMinusExpr);
			}

			//Constant
			const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
			if (numExpr){
				long double myDouble = numExpr->double_value();
				int nominator, denominator;
				simpleConvertToRational(myDouble, nominator, denominator);
				return liftedCVC4Problem->em.mkConst(Rational(nominator, denominator));
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
					return liftedCVC4Problem->em.mkConst(Rational(nominator, denominator));
				}
				int index = liftedCVC4Problem->getVariableIndex(pne2->getStateID(), operatorId, significantTimepoint);
				return liftedCVC4Problem->variableExpr[index];
			}
			CANT_HANDLE("can't handle One expression in converting to CVC4EXPR");
			return Expr();
		}

		static void simpleConvertToRational (double input, int &nominator, int &denominator){
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



#endif /* LIFTEDCVC4PROBLEM_H_ */
