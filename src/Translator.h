
#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "CVC4Problem.h"
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "MyAnalyzer.h"
#include "NumericRPG.h"
#include "SketchyPlan.h"
#include <ptree.h>

using namespace VAL;
using namespace Inst;




class Translator {
private:
	CVC4Problem *smtProblem;
	MyAnalyzer *myAnalyzer;
	NumericRPG *numericRPG;

	static vector <Expr> baseSATProblem;
	static vector <Expr> goals;

	//Prepare "baseSATproblem" and "goals" vectors for a specified length
	void prepare (int length);

	//Translate initial state of planning problem to SAT problem
	void addInitialState();

	//Add goals to the SAT problem
	void addGoals (int significantTimePoint);


	//Insert actions' conditions for the specified time point in SAT problem
	void addActions (int significantTimePoint);


	//Insert Explanatory Axioms which is needed for SAT problem
	void addExplanatoryAxioms (int significantTimePoint);

	//Insert action mutex to the SAT problem
	void addActionMutex (int significantTimePoint);

	//Insert the sketchy plan to the SAT problem
	void addSkechyPlan (SketchyPlan *sketchyPlan);


public:
	Translator (CVC4Problem *smtProblem, MyAnalyzer *myAnalyzer, NumericRPG *numericRPG): smtProblem(smtProblem), myAnalyzer(myAnalyzer), numericRPG(numericRPG)
	{
		if (baseSATProblem.size() == 0){
			smtProblem->clearAssertionList();
			addInitialState();
			baseSATProblem.push_back(smtProblem->getAssertions());
		}
		prepare(25);
	}

	bool solve (int length, SketchyPlan *sketchyPlan);

	virtual ~Translator(){}

private:

	void addSimpleEffectList (polarity plrty, const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env, int significantTimePoint, int actionID = -1);

	void addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, int actionID = -1);

	void addEffectList (const effect_lists *effects, FastEnvironment *env, int significantTimePoint, int actionId = -1);

	void addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, int actionId = -1);


};

#endif /* TRANSLATOR_H_ */
