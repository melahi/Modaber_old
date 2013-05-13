#ifndef PROBLEMPRINTER_H_
#define PROBLEMPRINTER_H_

#include <iostream>
#include <sstream>
#include <cmath>
#include <ptree.h>
#include <algorithm>
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

	void myDisplayLiteral (const polarity plrty, const proposition *prop){
		Literal lit = Literal(prop, env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		cout << '\t';
		if (plrty == E_NEG){
			cout << "NOT ";
		}
		lit2->write(cout);
		cout << " , StateID: " << lit2->getStateID() << ", GlobalID: " << lit2->getGlobalID();
		cout << endl;
	}

	string convertExpressionToString (const expression *expr){
		string ret;
		char exprOperator;
		const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
		if (binary){
			if (dynamic_cast <const plus_expression* > (expr)){
				exprOperator = '+';
			}else if (dynamic_cast<const minus_expression *> (expr)){
				exprOperator = '-';
			}else if (dynamic_cast<const mul_expression *> (expr)) {
				exprOperator = '*';
			}else if (dynamic_cast<const div_expression *> (expr)){
				exprOperator = '/';
			}else{
				CANT_HANDLE("binary_expression");
			}
			ret = convertExpressionToString(binary->getLHS());
			ret += exprOperator;
			ret += convertExpressionToString(binary->getRHS());
			return ret;
		}
		const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
		if (uMinus){
			ret = "- " + convertExpressionToString(uMinus->getExpr());
			return ret;
		}
		const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
		if (numExpr){
			long double myDouble = numExpr->double_value();
			int nominator, denominator;
			simpleConvertToRational(myDouble, nominator, denominator);
			ostringstream oss;
			oss << nominator << " / " << denominator;
			return oss.str();
		}
		const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
		if (functionTerm){
			PNE pne = PNE(functionTerm, env);
			PNE *pne2 = instantiatedOp::findPNE(&pne);
			ostringstream oss;
			oss << "[ ";
			pne2->write(oss);
			oss << ", StateID: " << pne2->getStateID() << ", GlobalID: " << pne2->getGlobalID() << " ]";
			return oss.str();
		}
		CANT_HANDLE("Expression");
		return ret;
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

	void myActionsPrint(){
		OpStore::iterator iter, itEnd;
		iter = instantiatedOp::opsBegin();
		itEnd = instantiatedOp::opsEnd();
		for (;iter != itEnd; iter++){
			cout << "Action " << (*iter)->getID() << ", "; (*iter)->write(cout); cout << endl;
			const operator_ *oper = (*iter)->forOp();
			FastEnvironment *env = (*iter)->getEnv();
			cout << "----------  Precondition --------------" << endl;
			MyGoalPrint myGoalPrint (env);
			myGoalPrint(oper->precondition);
			cout << "----------    Effects    --------------" << endl;
			MyEffectPrint myEffectPrint (env);
			myEffectPrint (oper->effects);
			cout << endl << endl;
		}
		cout << "END" << endl;
	}


	void myInitialStatePrint(){
		FastEnvironment env(0);
		MyEffectPrint myEffectPrint (&env);
		myEffectPrint(current_analysis->the_problem->initial_state);
	}

	void myGoalStatePrint(){
		FastEnvironment env(0);
		MyGoalPrint myGoalPrint(&env);
		myGoalPrint (current_analysis->the_problem->the_goal);
	}


public:

	ProblemPrinter();

	void printProblem(){
		cout << "###########  INITIAL STATE  ####################" << endl;
	 	myInitialStatePrint();
		cout << "##############  ACTIONS  #######################" << endl;
		myActionsPrint();
	 	cout << "###############  GOALS  ########################" << endl;
	 	myGoalStatePrint();
	}

	virtual ~ProblemPrinter();
};

#endif /* PROBLEMPRINTER_H_ */
