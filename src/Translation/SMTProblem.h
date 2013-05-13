//In the name of God


#ifndef SMTINTERFACE_H_
#define SMTINTERFACE_H_

#include "tsat/parser/ParserClasses.h"
using namespace MyParser;

class SMTProblem {
public:

	//Start to build new clause for SMT problem
	virtual void startNewClause() = 0;

	//By calling this function, you mean the clause is already built and it should be inserted to the SMT problem
	virtual void endClause() = 0;

	//Add new boolean condition to the building clause
	virtual void addConditionToCluase(int proposition, int significantTimePoint, bool polarity) = 0;
	//Add new action to the building clause
	virtual void addActionToClause (int action, int significantTimePoint, bool polarity) = 0;

	//Add new numerical condition to the building clause
	virtual void addConditionToCluase(PComparisonExpression* numericalCondition, int significantTimePoint,bool polarity) = 0;
	//Add new numerical assignment to the building clause
	virtual void addConditionToCluase(PAssignment* numericalAssignme, int significantTimePoint, bool polarity) = 0;

	//Solving the created problem
	virtual void solve() = 0;

	virtual ~SMTProblem(){};

};

#endif /* SMTINTERFACE_H_ */
