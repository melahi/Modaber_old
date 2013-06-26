

#include "CVC4Problem.h"
#include "VALfiles/parsing/ptree.h"

#include "sstream"


using namespace VAL;
using namespace std;


//Declaration for static variables
vector <double> CVC4Problem::initialValue;
ExprManager CVC4Problem::em;
vector <Expr> CVC4Problem::variableExpr;
vector <Expr> CVC4Problem::propositionExpr;
vector <Expr> CVC4Problem::actionExpr;
unsigned int CVC4Problem::maximumSignificantTimePoint;




void CVC4Problem::updateInitialValues (){
	if (CVC4Problem::initialValue.size() > 0){
		//initialValue vector has been initialized so we can return!
		return;
	}

	pc_list<assignment*>::const_iterator it = current_analysis->the_problem->initial_state->assign_effects.begin();
	pc_list<assignment*>::const_iterator itEnd = current_analysis->the_problem->initial_state->assign_effects.end();
	CVC4Problem::initialValue.resize(current_analysis->the_problem->initial_state->assign_effects.size());    //we assume in the initial state the value of every function (variable) has been declared
	FastEnvironment env(0);

	for (; it != itEnd; ++it){
		PNE pne ((*it)->getFTerm(), &env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		const num_expression *numExpr = dynamic_cast <const num_expression *>((*it)->getExpr());
		if (pne2 && numExpr && (*it)->getOp() == E_ASSIGN){
			CVC4Problem::initialValue[pne2->getGlobalID()] = numExpr->double_value();
		}else{
			CANT_HANDLE("Can't find Some Initial Value ")
		}
	}
	return;
}


void CVC4Problem::guaranteeSize (unsigned int nSignificantTimePoint){

	while (maximumSignificantTimePoint < nSignificantTimePoint){
		int size;

		//Creating variable expressions
		Type real = em.realType();
		size = variableExpr.size();
		for (int i = 0; i < nVariables; i++){
//			ostringstream oss;
//			oss << "variable(" << (size + i) % nVariables << "," << (size + i) / nVariables << ")";
//			variableExpr.push_back(em.mkVar(oss.str(),real));
			variableExpr.push_back(em.mkVar(real));
		}


		//Creating boolean expressions

		//For proposition
		Type boolean = em.booleanType();
		size = propositionExpr.size();
		for (int i = 0; i < nProposition; i++){
//			ostringstream oss;
//			oss << "proposition(" << (size + i) % nProposition << "," << (size + i) / nProposition << ")";
//			propositionExpr.push_back(em.mkVar(oss.str(), boolean));
			propositionExpr.push_back(em.mkVar(boolean));
		}


		//For action

		if (size){

			//		@TODO: for now we doesn't target temporal problems, perhaps for future it will be not bad to handle temporal problems!
			//		size = nSignificantTimePoint * nAction * 3;

			size = actionExpr.size();
			for (int i = 0; i < nAction; i++){
//				ostringstream oss;
//				oss << "action(" << (size + i) % nAction << "," << (size + i) / nAction << ")";
//				actionExpr.push_back(em.mkVar (oss.str(), boolean));
				actionExpr.push_back(em.mkVar (boolean));

			}

		}
		maximumSignificantTimePoint++;
	}
}

void CVC4Problem::initialization(){

	CVC4Problem::updateInitialValues();

	smt.setOption("produce-models", SExpr("true"));
	smt.setOption("check-models", SExpr("false"));		//In the case of debugging we can turn it to "true"
	smt.setOption("interactive-mode", SExpr("false"));		//In the case of debugging we can turn it to "true"
	smt.setOption("produce-assignments", SExpr("true"));
	smt.setOption("verbosity", SExpr("1073741823"));
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
}

//By calling this function, you mean the clause is already built and it should be inserted to the SMT problem
void CVC4Problem::endClause(){
	if (buildingClause.size() == 0)
		return;
	if (buildingClause.size() == 1){
		assertions.push_back(buildingClause[0]);
		return;
	}
	assertions.push_back(em.mkExpr(kind::OR, buildingClause));
	return;
}



void CVC4Problem::addLiteral ( polarity plrty, const proposition *prop, FastEnvironment *env, int significantTimePoint){
	Literal lit (prop, env);
	Literal *lit2 = instantiatedOp::findLiteral(&lit);
	if (lit2->getStateID() == -1)
		return;
	addConditionToCluase(lit2->getStateID(), significantTimePoint, (plrty == E_POS));
}

//Add new boolean condition to the building clause
void CVC4Problem::addConditionToCluase(int propostion, int significantTimePoint, bool polarity){
	int index = getPropositionIndex(propostion, significantTimePoint);
	if (polarity){
		buildingClause.push_back(propositionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, propositionExpr[index]));
	}
}

//Add new action to the building clause
void CVC4Problem::addActionToClause (int actionId, int significantTimePoint, bool polarity){
	int index = getActionIndex(actionId, significantTimePoint);
	if (polarity){
		buildingClause.push_back(actionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, actionExpr[index]));
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

void CVC4Problem::AddEqualityCondition (int variableId1, int significantTimePoint1, int variableId2, int significantTimePoint2){
	int variableIndex1 = getVariableIndex(variableId1, significantTimePoint1);
	int variableIndex2 = getVariableIndex(variableId2, significantTimePoint2);
	Expr equalityCondition = em.mkExpr(kind::EQUAL, variableExpr[variableIndex1], variableExpr[variableIndex2]);
	buildingClause.push_back(equalityCondition);
}

bool CVC4Problem::solve(const Expr &assertExpr){

	// TODO: For now, we don't considered processing time, we should consider it ASAP
	// TODO: We need statistical information

	cout << "Start to try solving the problem" << endl;
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
		return true;
		break;
	case Result::UNSAT:
		cout << "The problem is not satisfiable!!!" << endl;
		break;
	default:
		cout << "The result is neither \"SAT\" nor \"UNSAT\"!!" << endl;
		break;
	}
	return false;
}

void CVC4Problem::print(){
	vector <Expr> assertions = smt.getAssertions();
	for (size_t i = 0; i < assertions.size(); i++){
		if (i){
			cout << "AND   ";
		}
		assertions[i].toStream(cout);
		cout << endl;
	}
}

bool CVC4Problem::isActionUsed (int actionId, int significantTimePoint){
	int actionIndex = getActionIndex(actionId, significantTimePoint);
	bool isUsed = smt.getValue(actionExpr[actionIndex]).getConst<bool>();
	return isUsed;
}

void CVC4Problem::push(){
	smt.push();
}

void CVC4Problem::pop(){
	smt.pop();
}

void CVC4Problem::assertExpression(const Expr &e){
	if (!e.isNull()) {
		assertions.push_back(e);
	}
}

Expr CVC4Problem::andAssertionList (int begin, int end){
	if (end - begin <= em.maxArity(kind::AND)){
		return em.mkExpr(kind::AND, vector <Expr> (assertions.begin() + begin, assertions.begin() + end));
	}
	int middle = (begin + end) / 2;
	return em.mkExpr(kind::AND, andAssertionList(begin, middle), andAssertionList(middle, end));
}


Expr CVC4Problem::getAssertions(){
	if (assertions.size() == 0){
		return Expr();
	}
	if (assertions.size() == 1){
		return assertions[0];
	}
	return smt.simplify(andAssertionList(0, assertions.size()));
}

void CVC4Problem::clearAssertionList(){
	assertions.clear();
}

CVC4Problem::~CVC4Problem(){
}


//find and return the index of corresponding PVariableExpression in the variableExpr array
int CVC4Problem::getVariableIndex (int variableStateId, int significantTimePoint){
	return significantTimePoint * nVariables + variableStateId;
}

//find and return the index of corresponding proposition in the propositionExpr array
inline int CVC4Problem::getPropositionIndex (int proposition, int significantTimePoint){
return significantTimePoint * nProposition + proposition;
}

//find and return the index of corresponding action in the actionExpr array
inline int CVC4Problem::getActionIndex (int action, int significantTimePoint){
	return significantTimePoint * nAction + action;
}







