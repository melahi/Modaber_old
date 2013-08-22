

#include "CVC4Problem.h"
#include "VALfiles/parsing/ptree.h"


#include "sstream"
#include <limits>


using namespace VAL;
using namespace std;


//Declaration for static variables
ExprManager CVC4Problem::em;
vector <Expr *> CVC4Problem::variableExpr;
vector <Expr *> CVC4Problem::propositionExpr;
vector <Expr *> CVC4Problem::actionExpr;
unsigned int CVC4Problem::maximumSignificantTimePoint;






void CVC4Problem::guaranteeSize (unsigned int nSignificantTimePoint){

	if (maximumSignificantTimePoint < nSignificantTimePoint){

		variableExpr.resize (nSignificantTimePoint * nVariables);
		propositionExpr.resize (nSignificantTimePoint * nProposition);
		actionExpr.resize((nSignificantTimePoint - 1) * nAction);
		maximumSignificantTimePoint = nSignificantTimePoint;
	}

//	while (maximumSignificantTimePoint < nSignificantTimePoint){
//		int size;
//
//		//Creating variable expressions
//		Type real = em.realType();
//		size = variableExpr.size();
//		for (int i = 0; i < nVariables; i++){
////			ostringstream oss;
////			oss << "[";
////			myProblem.variables[i].write(oss);
////			oss << ", " << (size + i) / nVariables << "]";
////			variableExpr.push_back(em.mkVar(oss.str(),real));
//			variableExpr.push_back(em.mkVar(real));
//		}
//
//
//		//Creating boolean expressions
//
//		//For proposition
//		Type boolean = em.booleanType();
//		size = propositionExpr.size();
//		for (int i = 0; i < nProposition; i++){
////			ostringstream oss;
////			oss << "[";
////			myProblem.propositions[i].write(oss);
////			oss << ", " << (size + i) / nProposition << "]";
////			propositionExpr.push_back(em.mkVar(oss.str(), boolean));
//			propositionExpr.push_back(em.mkVar(boolean));
//		}
//
//
//		//For action
//
//		if (size){
//
////			@TODO: for now we doesn't target temporal problems, perhaps for future it will be not bad to handle temporal problems!
////			size = nSignificantTimePoint * nAction * 3;
//
//			size = actionExpr.size();
//			for (int i = 0; i < nAction; i++){
////				ostringstream oss;
////				oss << "[";
////				myProblem.actions[i].write(oss);
////				oss << ", " << (size + i) / nAction << "]";
////				actionExpr.push_back(em.mkVar (oss.str(), boolean));
//				actionExpr.push_back(em.mkVar (boolean));
//
//			}
//
//		}
//		maximumSignificantTimePoint++;
//	}
}

void CVC4Problem::initialization(){

	smt.setOption("produce-models", SExpr("true"));
	smt.setOption("check-models", SExpr("false"));		//In the case of debugging we can turn it to "true"
	smt.setOption("interactive-mode", SExpr("true"));		//In the case of debugging we can turn it to "true"
	smt.setOption("produce-assignments", SExpr("true"));
//	smt.setOption("verbosity", SExpr("1073741823"));
	smt.setOption("verbosity", SExpr("0"));
	smt.setOption("incremental", SExpr("true"));

	maximumSignificantTimePoint = 0;

	guaranteeSize(1);
}



CVC4Problem::CVC4Problem (int nVariables, int nProposition, int nAction): smt(&em), nVariables(nVariables), nProposition(nProposition), nAction(nAction) {
	initialization();
}


//Start to build new clause for SMT problem
void CVC4Problem::startNewClause(){
	buildingClause.clear();
	ignoreCluase = false;
}

//By calling this function, you mean the clause is already built and it should be inserted to the SMT problem
void CVC4Problem::endClause(){
	if (ignoreCluase ){
		return;
	}
	if (buildingClause.size() == 0)
		return;
	if (buildingClause.size() == 1){
		if (permanentChange){
			smt.assertFormula(buildingClause[0]);
		}else{
			assertions.push_back(buildingClause[0]);
		}
		return;
	}
	if (permanentChange){
		smt.assertFormula(em.mkExpr(kind::OR, buildingClause));
	}else{
		assertions.push_back(em.mkExpr(kind::OR, buildingClause));
	}
	return;
}



void CVC4Problem::addLiteral ( polarity plrty, const proposition *prop, FastEnvironment *env, int significantTimePoint){
	Literal lit (prop, env);
	Literal *lit2 = instantiatedOp::findLiteral(&lit);

	if (!lit2){
		CANT_HANDLE("Warning: can't find some literal!!!");
		lit2->write(cerr);
		ignoreCluase = true;
		return;
	}
	if (lit2->getStateID() == -1){
		ignoreCluase = true;
		return;
	}
	addConditionToCluase(lit2->getStateID(), significantTimePoint, (plrty == E_POS));
}

//Add new boolean condition to the building clause
void CVC4Problem::addConditionToCluase(int propostion, int significantTimePoint, bool polarity){
	int index = getPropositionIndex(propostion, significantTimePoint);
	if (polarity){
		buildingClause.push_back(*propositionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, *propositionExpr[index]));
	}
}


void CVC4Problem::AddConditionToCluase(const MyAtom *atom, int significantTimePoint, bool polarity){
	const MyProposition *myProposition = dynamic_cast <const MyProposition*> (atom);
	const MyValue *myValue = dynamic_cast <const MyValue*> (atom);

	if (myProposition){
		int index = getPropositionIndex(myProposition->originalLiteral->getStateID(), significantTimePoint);
		if (polarity){
			buildingClause.push_back(*propositionExpr[index]);
		}else{
			buildingClause.push_back(em.mkExpr(kind::NOT, *propositionExpr[index]));
		}
	}else{
		AddEqualityCondition(myValue->variable->originalPNE->getStateID(), significantTimePoint, myValue->value, polarity);
	}
}



//Add new action to the building clause
void CVC4Problem::addActionToClause (int actionId, int significantTimePoint, bool polarity){
	int index = getActionIndex(actionId, significantTimePoint);
	if (polarity){
		buildingClause.push_back(*actionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, *actionExpr[index]));
	}
}

//Add new numerical condition to the building clause
void CVC4Problem::AddConditionToCluase(const comparison* numericalCondition, FastEnvironment *env, int significantTimePoint){
	ExpressionConvertor myConvertor(env, this, significantTimePoint);
	Expr left = myConvertor.convertExpressionToCVC4Expr(numericalCondition->getLHS());
	Expr right = myConvertor.convertExpressionToCVC4Expr(numericalCondition->getRHS());
	Kind operatorKind;
	switch(numericalCondition->getOp()){
	case E_GREATER:
		operatorKind = kind::GT;
		break;
	case E_GREATEQ:
		operatorKind = kind::GEQ;
		break;
	case E_LESS:
		operatorKind = kind::LT;
		break;
	case E_LESSEQ:
		operatorKind = kind::LEQ;
		break;
	case E_EQUALS:
		operatorKind = kind::EQUAL;
		break;
	default:
		CANT_HANDLE ("We don't know the operator kind of numerical condition!!!");
		exit(0);
	}
	buildingClause.push_back(em.mkExpr (operatorKind, left, right));
}

//Add new numerical assignment to the building clause
void CVC4Problem::AddConditionToCluase(const assignment* numericalAssignment, FastEnvironment *env, int significantTimePoint){
	ExpressionConvertor variableConvertor(env, this, significantTimePoint);
	Expr variable = variableConvertor.convertExpressionToCVC4Expr(numericalAssignment->getFTerm());
	if (variable.isConst()){
		//This case is happening just in initial case when initial state determine the value of a static variable
		return;
	}
	ExpressionConvertor expressionConvertor(env, this, significantTimePoint - 1);
	Expr result = expressionConvertor.convertExpressionToCVC4Expr(numericalAssignment->getExpr());

	Kind assignmentOperator = kind::EQUAL;
	switch (numericalAssignment->getOp()){
	case E_INCREASE:
		assignmentOperator = kind::PLUS;
		break;
	case E_DECREASE:
		assignmentOperator = kind::MINUS;
		break;
	case E_SCALE_UP:
		assignmentOperator = kind::MULT;
		break;
	case E_SCALE_DOWN:
		assignmentOperator = kind::DIVISION;
		break;
	case E_ASSIGN:
		break;
	case E_ASSIGN_CTS:
		cerr << "Oops!!!, I don't know what is \"E_ASSIGN_CTS\"" << endl;
		exit(1);
		break;
	default:
		cerr << numericalAssignment->getOp() << endl;
		cerr << "I think the program should never reach at this line, BTW we just was processing a numerical assignment!" << endl;
		exit (1);
	}

	if (assignmentOperator != kind::EQUAL){
		Expr variableInPreviousTime = expressionConvertor.convertExpressionToCVC4Expr(numericalAssignment->getFTerm());
		result = em.mkExpr(assignmentOperator, variableInPreviousTime, result);
	}

	buildingClause.push_back(em.mkExpr(kind::EQUAL, variable, result));
}

void CVC4Problem::AddEqualityCondition (int variableId1, int significantTimePoint1, int variableId2, int significantTimePoint2, bool polarity){
	int variableIndex1 = getVariableIndex(variableId1, significantTimePoint1);
	int variableIndex2 = getVariableIndex(variableId2, significantTimePoint2);
	Kind myKind;
	if (polarity){
		myKind = kind::EQUAL;
	}else{
		myKind = kind::DISTINCT;
	}
	Expr myCondition = em.mkExpr(myKind, *variableExpr[variableIndex1], *variableExpr[variableIndex2]);
	buildingClause.push_back(myCondition);
}

void CVC4Problem::AddEqualityCondition (int variableId1, int significantTimePoint1, double value, bool polarity){
	int variableIndex1 = getVariableIndex(variableId1, significantTimePoint1);

	int nominator, denominator;
	ExpressionConvertor::simpleConvertToRational(value, nominator, denominator);
	Expr valExpr = em.mkConst(Rational(nominator, denominator));
	Kind myKind;
	if (polarity){
		myKind = kind::EQUAL;
	}else{
		myKind = kind::DISTINCT;
	}
	Expr myCondtion = em.mkExpr(myKind, *variableExpr[variableIndex1], valExpr);
	buildingClause.push_back(myCondtion);
}


double CVC4Problem::solve(const Expr &assertExpr){

	// TODO: For now, we don't considered processing time, we should consider it ASAP
	// TODO: We need statistical information

//	cout << "Start to try solving the problem" << endl;
//	smt.setResourceLimit(resourceLimit);
	Result result = smt.checkSat(assertExpr);



	/*      For debug
	Statistics myStatistics = smt.getStatistics();
	cout << "*****************************Start****************************" << endl;
	myStatistics.flushInformation(cout);
	cout << "******************************End*****************************" << endl;
	*/


	switch (result.isSat()){
	case Result::SAT:
		cout << "OH yeay!, the problem is solved" << endl;
		return numeric_limits <double>::max();
		break;
	case Result::UNSAT:
//		cout << "The problem is not satisfiable!!!" << endl;
		break;
	default:
//		cout << "The result is neither \"SAT\" nor \"UNSAT\"!!" << endl;
		break;
	}
//	cout << "Number of conflicts: " << smt.getStatistic("sat::conflicts").getValue() << endl;
//	cout << "Number of decision: " << smt.getStatistic("sat::decisions").getValue() << endl;
//	istringstream sin (smt.getStatistic("sat::decisions").getValue());
//	double ret;
//	unsigned long int newNumberOfDecision;
//	sin >> newNumberOfDecision;
//	ret = newNumberOfDecision - lastNumberOfDecision;
//	lastNumberOfDecision = newNumberOfDecision;
	return 0;
}

void CVC4Problem::print(){
	vector <Expr> assertions = smt.getAssertions();
	print (assertions);
}
int mine = 0;
void CVC4Problem::print(vector <Expr> &expression){
	mine++;
	cout << mine << endl;
	for (size_t i = 0; i < expression.size(); i++){
		if (expression[i].getKind() == kind::AND){
			vector <Expr> child = expression[i].getChildren();
			print(child);
		}else{
			expression[i].toStream(cout);
			cout << endl;
		}
	}
	mine--;
}


bool CVC4Problem::isActionUsed (int actionId, int significantTimePoint){
	int actionIndex = getActionIndex(actionId, significantTimePoint);
	bool isUsed = smt.getValue(*actionExpr[actionIndex]).getConst<bool>();
	return isUsed;
}

void CVC4Problem::push(){
	smt.push();
}

void CVC4Problem::pop(){
	smt.pop();
}

void CVC4Problem::insertAssertion(const Expr &e){
	if (!e.isNull()) {
		assertions.push_back(e);
	}
}

Expr CVC4Problem::andAssertionList (unsigned int begin, unsigned int end){
	if (end - begin <= em.maxArity(kind::AND)){
		return em.mkExpr(kind::AND, vector <Expr> (assertions.begin() + begin, assertions.begin() + end));
	}
	unsigned int middle = (begin + end) / 2;
	return em.mkExpr(kind::AND, andAssertionList(begin, middle), andAssertionList(middle, end));
}


Expr CVC4Problem::getAssertions(){
	if (assertions.size() == 0){
		return Expr();
	}
	if (assertions.size() == 1){
		return assertions[0];
	}
//	Expr ret = smt.simplify(andAssertionList(0, assertions.size()));
	Expr ret = andAssertionList(0, assertions.size());
	return ret;
}

void CVC4Problem::clearAssertionList(){
	assertions.clear();
}

CVC4Problem::~CVC4Problem(){
	for (unsigned int i = 0; i < variableExpr.size(); i++){
		delete( variableExpr[i] );
	}
	for (unsigned int i = 0; i < propositionExpr.size(); i++){
		delete( propositionExpr[i] );
	}
	for (unsigned int i = 0; i < actionExpr.size(); i++){
		delete( actionExpr[i] );
	}
}


//find and return the index of corresponding PVariableExpression in the variableExpr array
int CVC4Problem::getVariableIndex (int variableStateId, int significantTimePoint){
	int ret = significantTimePoint * nVariables + variableStateId;
	if (variableExpr[ret] == NULL){
		Type real = em.realType();
//		ostringstream oss;
//		oss << "[";
//		myProblem.variables[i].write(oss);
//		oss << ", " << (size + i) / nVariables << "]";
//		variableExpr.push_back(em.mkVar(oss.str(),real));
		variableExpr[ret] = new Expr(em.mkVar(real));
	}
	return ret;
}

//find and return the index of corresponding proposition in the propositionExpr array
inline int CVC4Problem::getPropositionIndex (int proposition, int significantTimePoint){
	int ret = significantTimePoint * nProposition + proposition;
	if (propositionExpr[ret] == NULL){
		Type boolean = em.booleanType();
//		ostringstream oss;
//		oss << "[";
//		myProblem.propositions[i].write(oss);
//		oss << ", " << (size + i) / nProposition << "]";
//		propositionExpr.push_back(em.mkVar(oss.str(), boolean));
		propositionExpr[ret] = new Expr(em.mkVar(boolean));
	}
	return ret;
}

//find and return the index of corresponding action in the actionExpr array
inline int CVC4Problem::getActionIndex (int action, int significantTimePoint){
	int ret = significantTimePoint * nAction + action;
	if (actionExpr[ret] == NULL){
		Type boolean = em.booleanType();
//		ostringstream oss;
//		oss << "[";
//		myProblem.actions[i].write(oss);
//		oss << ", " << (size + i) / nAction << "]";
//		actionExpr.push_back(em.mkVar (oss.str(), boolean));
		actionExpr[ret] = new Expr(em.mkVar (boolean));
	}
	return ret;
}







