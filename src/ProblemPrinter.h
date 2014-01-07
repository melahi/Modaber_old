#ifndef PROBLEMPRINTER_H_
#define PROBLEMPRINTER_H_

#include <iostream>
#include <sstream>
#include <cmath>
#include <ptree.h>
#include <algorithm>
#include <map>
#include <vector>

#include "VALfiles/SimpleEval.h"
#include "VALfiles/instantiation.h"

using namespace std;
using namespace VAL;
using namespace Inst;

#define CANT_HANDLE(x) cerr << "********************************" << x << "********************************" << endl;







class MyPrint{

protected:
	FastEnvironment *env;
public:
	MyPrint(FastEnvironment *env):env(env) {};

	void myDisplayLiteral (const polarity plrty, const proposition *prop);

	string convertExpressionToString (const expression *expr);

	void simpleConvertToRational (double input, int &nominator, int &denominator);
};

class MyGoalPrint : public MyPrint{

public:

	MyGoalPrint (FastEnvironment *env):MyPrint(env){};
	void operator() (const goal *gl){
		const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
		if (simple){
			myDisplayLiteral(simple->getPolarity(), simple->getProp());
			return;
		}
		const comparison *comp = dynamic_cast<const comparison*> (gl);
		if (comp){
			string result = convertExpressionToString(comp->getLHS());
			switch (comp->getOp()){
			case E_GREATER:
				result += " > ";
				break;
			case E_GREATEQ:
				result += " >= ";
				break;
			case E_LESS:
				result += " < ";
				break;
			case E_LESSEQ:
				result += " > ";
				break;
			case E_EQUALS:
				result += " == ";
				break;
			default:
				CANT_HANDLE("operator");
				break;
			}
			result += convertExpressionToString(comp->getRHS());
			cout << '\t' << result << endl;
			return;
		}
		const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
		if (conjunctive){
			const goal_list *goalList = conjunctive->getGoals();
			for_each(goalList->begin(), goalList->end(), *this);
			return;
		}
		const preference *thePreference = dynamic_cast <const preference *> (gl);
		if (thePreference){
			cout << "--- Preference: " << thePreference->getName() << " ---"<< endl;
			(*this)(thePreference->getGoal());
			return;
		}
		CANT_HANDLE("GOAL");
	}

};



class MyEffectPrint : public MyPrint{

public:

	MyEffectPrint (FastEnvironment *env):MyPrint(env){};

	void simpleEffectList (polarity plrty, pc_list<simple_effect*> &simpleEffectList){
		pc_list<simple_effect*>::const_iterator it = simpleEffectList.begin();
		pc_list<simple_effect*>::const_iterator itEnd = simpleEffectList.end();
		for (; it != itEnd; ++it){
			myDisplayLiteral(plrty, (*it)->prop);
		}
	}

	void assignmentList (pc_list<assignment*> &assignmentEffects){
		pc_list<assignment*>::const_iterator it = assignmentEffects.begin();
		pc_list<assignment*>::const_iterator itEnd = assignmentEffects.end();
		for (; it != itEnd; ++it){
			string result = convertExpressionToString((expression *) ((*it)->getFTerm()));
			switch ((*it)->getOp()){
			case E_ASSIGN:
				result += " = ";
				break;
			case E_INCREASE:
				result += " += ";
				break;
			case E_DECREASE:
				result += " -= ";
				break;
			case E_SCALE_UP:
				result += " *= ";
				break;
			case E_SCALE_DOWN:
				result += " /= ";
				break;
			case E_ASSIGN_CTS:
				result += " ??? ";
				break;
			}
			result += convertExpressionToString((*it)->getExpr());
			cout << '\t' << result << endl;
		}

	}
	void operator() (effect_lists *effects){
		cout << "Simple Effects:" << endl;
		simpleEffectList(E_POS, effects->add_effects);
		simpleEffectList(E_NEG, effects->del_effects);
		cout << "Assignment Effects:" << endl;
		assignmentList(effects->assign_effects);
		if ((!effects->forall_effects.empty()) || (!effects->cond_effects.empty()) || (!effects->cond_assign_effects.empty()) || (!effects->timed_effects.empty())){
			CANT_HANDLE("Some kinds of Effects");
		}
	}

};






class ProblemPrinter {

private:

	void myActionsPrint();

	void myInitialStatePrint();

	void myGoalStatePrint();

	void myTypesAndObjectsPrint();


public:

	ProblemPrinter(){}

	void printProblem();

	virtual ~ProblemPrinter(){}
};

#endif /* PROBLEMPRINTER_H_ */
