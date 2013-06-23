
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
public:
	Translator (CVC4Problem *smtProblem, MyAnalyzer *myAnalyzer, NumericRPG *numericRPG): smtProblem(smtProblem), myAnalyzer(myAnalyzer), numericRPG(numericRPG) {};

	void setSMTProblem (CVC4Problem *smtProblem) {this->smtProblem = smtProblem;}
	CVC4Problem *getSMTProblem () {return this->smtProblem;}

	//Translate initial state of planning problem to SMT problem
	void addInitialState();

	//Add goals to the smt problem
	void addGoals (int significantTimePoint);


	//Insert actions' conditions for the specified time point in smt problem
	void addActions (int significantTimePoint);


	//Insert Explanatory Axioms which is needed for SMT problem
	void addExplanatoryAxioms (int significantTimePoint);

	//Insert action mutex to the SMT problem
	void addActionMutex (int significantTimePoint);

	//Insert the sketchy plan to the SMT problem
	void addSkechyPlan (SketchyPlan *sketchyPlan);


	virtual ~Translator(){}

private:

	void addSimpleEffectList (polarity plrty, const pc_list<simple_effect*> &simpleEffectList, FastEnvironment *env, int significantTimePoint, int actionID = -1);

	void addAssignmentList (const pc_list <assignment *> &assignmentEffects, FastEnvironment *env, int significantTimePoint, int actionID = -1);

	void addEffectList (const effect_lists *effects, FastEnvironment *env, int significantTimePoint, int actionId = -1);

	void addGoal (const goal *gl, FastEnvironment *env, int significantTimePoint, int actionId = -1);


};

#endif /* TRANSLATOR_H_ */
