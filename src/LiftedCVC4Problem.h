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

	void addUnificationToClause (int unificationId, int significantTimePoint, bool polarity);

	//Add new boolean condition to the building clause
	void addConditionToCluase(int propostion, int operatorId, int significantTimePoint, bool polarity);

	void AddConditionToCluase(const MyProposition *myProposition, int operatorId, int significantTimePoint, bool polarity);

	//Add new numerical condition to the building clause
	void AddConditionToCluase(const comparison* numericalCondition, FastEnvironment *env, int operatorId, int significantTimePoint);
	void AddConditionToCluase(const expression *leftExpression, FastEnvironment *env, int operatorId, comparison_op compOp, double rightValue, int significantTimePoint);

	//Add new numerical assignment to the building clause
	void AddConditionToCluase(const assignment* numericalAssignment, FastEnvironment *env, int operatorId, int significantTimePoint);

	void AddEqualityCondition (Expr expr, double value);

	void AddEqualityCondition (int variableId1, int operatorId1, int significantTimePoint1, int variableId2, int operatorId2, int significantTimePoint2, bool polarity);

	void AddEqualityCondition (int variableId1, int operatorId, int significantTimePoint1, double value, bool polarity);


	double getExpressionValue (const expression *valExpr, FastEnvironment *env, int operatorId, int significantTimePoint);

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

	bool isUnificationUsed (int unificationId, int significantTimePoint);
	bool isPartialActionUsed(MyPartialAction *partialAction, int significantTimePoint);


	/* I don't know how to implement following function, perhaps it's not bad to learn it!
	CVC4Problem& operator=(const CVC4Problem&);
	*/

	void activePermanentChange() { permanentChange = true;}
	void inActivePermanentChange() { permanentChange = false;}
	virtual ~LiftedCVC4Problem();


	ExprManager em;
	SmtEngine smt;

	int nVariables;
	int nPropositions;
	int nPartialActions;
	int nUnifications;


	map <string, Expr> preferenceExpr;
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
	int getVariableIndex (unsigned int variableId, unsigned int operatorId, int significantTimePoint);

	Expr getPreferenceExpr (string name);


	//find and return the index of corresponding proposition in the propositionExpr array
	int getPropositionIndex (unsigned int propositionId, unsigned int operatorId, int significantTimePoint);

	//find and return the index of corresponding action in the actionExpr array
	int getPartialActionIndex (MyPartialAction *partialAction, int significantTimePoint);

	//find and return the index of corresponding action in the actionExpr array
	int getUnificationIndex (int unificationId, int significantTimePoint);


};



#endif /* LIFTEDCVC4PROBLEM_H_ */
