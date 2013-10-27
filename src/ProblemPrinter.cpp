#include "ProblemPrinter.h"

using namespace std;
using namespace VAL;
using namespace Inst;

void MyPrint::myDisplayLiteral (const polarity plrty, const proposition *prop){
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

string MyPrint::convertExpressionToString (const expression *expr){
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
void MyPrint::simpleConvertToRational (double input, int &nominator, int &denominator){
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

void ProblemPrinter::myActionsPrint(){
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


void ProblemPrinter::myInitialStatePrint(){
	FastEnvironment env(0);
	MyEffectPrint myEffectPrint (&env);
	myEffectPrint(current_analysis->the_problem->initial_state);
}

void ProblemPrinter::myGoalStatePrint(){
	FastEnvironment env(0);
	MyGoalPrint myGoalPrint(&env);
	myGoalPrint (current_analysis->the_problem->the_goal);
}

void ProblemPrinter::myTypesAndObjectsPrint() {
	map <VAL::pddl_type *, vector <VAL::const_symbol *> > objects;
	const_symbol_list::iterator it2, itEnd2;

	it2 = current_analysis->the_problem->objects->begin();
	itEnd2 = current_analysis->the_problem->objects->end();
	for (int i = 0; it2 != itEnd2; ++it2, ++i){
		if ((*it2)->type){
			objects[(*it2)->type].push_back(*it2);
		}else if ((*it2)->either_types){
			pddl_type_list::iterator it3, itEnd3;
			it3 = (*it2)->either_types->begin();
			itEnd3 = (*it2)->either_types->end();
			for (; it3 != itEnd3; ++it3){
				objects[*it3].push_back(*it2);
			}
		}else{
			objects[NULL].push_back (*it2);
		}
	}

	map <VAL::pddl_type *, vector <VAL::const_symbol *> >::iterator objIt, objItEnd;
	objIt = objects.begin();
	objItEnd = objects.end();

	cout << current_analysis->the_domain->types->size() << " Types: " << endl;
	for (int i = 0; objIt != objItEnd; ++objIt, ++i){
		cout << '\t' << i << ": " << (objIt)->first->getName() << endl;
		for (unsigned int j = 0; j < objIt->second.size(); ++j){
			cout << "\t\t" << j << ": " << objIt->second[j]->getName() << endl;
		}
	}
}



void ProblemPrinter::printProblem(){
	cout << "###########  INITIAL STATE  ####################" << endl;
	myInitialStatePrint();
	cout << "##############  ACTIONS  #######################" << endl;
	myActionsPrint();
	cout << "###############  GOALS  ########################" << endl;
	myGoalStatePrint();

	cout << "###############  Types & object  ########################" << endl;
	myTypesAndObjectsPrint();

}
