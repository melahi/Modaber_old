

#include "LiftedCVC4Problem.h"
#include "VALfiles/parsing/ptree.h"


#include "sstream"
#include <limits>


using namespace VAL;
using namespace std;





void LiftedCVC4Problem::guaranteeSize (unsigned int nSignificantTimePoint){

	if (maximumSignificantTimePoint < nSignificantTimePoint){
		variableExpr.resize (nSignificantTimePoint * nVariables);
		propositionExpr.resize (nSignificantTimePoint * nPropositions);
		partialActionExpr.resize((nSignificantTimePoint - 1) * nPartialActions);
		unificationExpr.resize((nSignificantTimePoint - 1) * nUnifications);
		maximumSignificantTimePoint = nSignificantTimePoint;
	}
}

void LiftedCVC4Problem::initialization(){

	smt.setOption("produce-models", SExpr("true"));
	smt.setOption("check-models", SExpr("false"));		//In the case of debugging we can turn it to "true"
	smt.setOption("interactive-mode", SExpr("false"));		//In the case of debugging we can turn it to "true"
	smt.setOption("produce-assignments", SExpr("true"));
	smt.setOption("verbosity", SExpr("1073741823"));
//	smt.setOption("verbosity", SExpr("0"));
	smt.setOption("incremental", SExpr("false"));
	smt.setLogic("QF_LIRA");

	maximumSignificantTimePoint = 0;

	guaranteeSize(1);


	Type boolean = em.booleanType();
//	ostringstream oss;
//	oss << "[ TRUE ]";
//	propositionExpr[ret] = Expr(em.mkVar(oss.str(), boolean));
	trueExpr = Expr(em.mkVar(boolean));
	falseExpr = em.mkExpr(kind::NOT, trueExpr);
	smt.assertFormula(trueExpr);
}



LiftedCVC4Problem::LiftedCVC4Problem (int nVariables, int nPropositions, int nPartialActions, int nUnifications): em(), smt(&em), nVariables(nVariables), nPropositions(nPropositions), nPartialActions(nPartialActions), nUnifications(nUnifications){
	initialization();
}


//Start to build new clause for SMT problem
void LiftedCVC4Problem::startNewClause(){
	buildingClause.clear();
	ignoreCluase = false;
}

//By calling this function, you mean the clause is already built and it should be inserted to the SMT problem
void LiftedCVC4Problem::endClause(){
	if (ignoreCluase){
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



void LiftedCVC4Problem::addLiteral ( polarity plrty, const proposition *prop, FastEnvironment *env, int operatorId, int significantTimePoint){
	Literal lit (prop, env);
	Literal *lit2 = instantiatedOp::findLiteral(&lit);

	if (!lit2){
		buildingClause.push_back(falseExpr);
		return;
	}
	if (lit2->getStateID() == -1){
		ignoreCluase = true;
		return;
	}
	addConditionToCluase(lit2->getStateID(), operatorId, significantTimePoint, (plrty == E_POS));
}

//Add new boolean condition to the building clause
void LiftedCVC4Problem::addConditionToCluase(int propostionId, int operatorId, int significantTimePoint, bool polarity){
	int index = getPropositionIndex(propostionId, operatorId, significantTimePoint);
	if (polarity){
		buildingClause.push_back(propositionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, propositionExpr[index]));
	}
}


void LiftedCVC4Problem::AddConditionToCluase(const MyProposition *myProposition, int operatorId, int significantTimePoint, bool polarity){
	int index = getPropositionIndex(myProposition->originalLiteral->getStateID(), operatorId, significantTimePoint);
	if (polarity){
		buildingClause.push_back(propositionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, propositionExpr[index]));
	}
}




void LiftedCVC4Problem::addPartialActionToClause (MyPartialAction *partialAction, int significantTimePoint, bool polarity){
	int index = getPartialActionIndex(partialAction, significantTimePoint);
	if (polarity){
		buildingClause.push_back(partialActionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, partialActionExpr[index]));
	}
}

//Add new numerical condition to the building clause
void LiftedCVC4Problem::AddConditionToCluase(const comparison* numericalCondition, FastEnvironment *env, int operatorId, int significantTimePoint){
	ExpressionConvertor myConvertor(env, this, operatorId, significantTimePoint);
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
void LiftedCVC4Problem::AddConditionToCluase(const assignment* numericalAssignment, FastEnvironment *env, int operatorId, int significantTimePoint){
	ExpressionConvertor variableConvertor(env, this, operatorId + 1, significantTimePoint);
	Expr variable = variableConvertor.convertExpressionToCVC4Expr(numericalAssignment->getFTerm());
	if (variable.isConst()){
		//This case is happening just in initial case when initial state determine the value of a static variable
		return;
	}
	ExpressionConvertor expressionConvertor(env, this, operatorId, significantTimePoint - 1);
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

void LiftedCVC4Problem::AddEqualityCondition (int variableId1, int operatorId1, int significantTimePoint1, int variableId2, int operatorId2, int significantTimePoint2, bool polarity){
	int variableIndex1 = getVariableIndex(variableId1, operatorId1, significantTimePoint1);
	int variableIndex2 = getVariableIndex(variableId2, operatorId2, significantTimePoint2);
	Kind myKind;
	if (polarity){
		myKind = kind::EQUAL;
	}else{
		myKind = kind::DISTINCT;
	}
	Expr myCondition = em.mkExpr(myKind, variableExpr[variableIndex1], variableExpr[variableIndex2]);
	buildingClause.push_back(myCondition);
}

void LiftedCVC4Problem::AddEqualityCondition (int variableId1, int operatorId, int significantTimePoint1, double value, bool polarity){
	int variableIndex1 = getVariableIndex(variableId1, operatorId, significantTimePoint1);

	int nominator, denominator;
	ExpressionConvertor::simpleConvertToRational(value, nominator, denominator);
	Expr valExpr = em.mkConst(Rational(nominator, denominator));
	Kind myKind;
	if (polarity){
		myKind = kind::EQUAL;
	}else{
		myKind = kind::DISTINCT;
	}
	Expr myCondtion = em.mkExpr(myKind, variableExpr[variableIndex1], valExpr);
	buildingClause.push_back(myCondtion);
}


bool LiftedCVC4Problem::solve(const Expr &assertExpr){

	// TODO: For now, we don't considered processing time, we should consider it ASAP
	// TODO: We need statistical information

//	cout << "Start to try solving the problem" << endl;
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
//		cout << "The problem is not satisfiable!!!" << endl;
		break;
	default:
//		cout << "The result is neither \"SAT\" nor \"UNSAT\"!!" << endl;
		break;
	}
	return false;
}

void LiftedCVC4Problem::print(){
	vector <Expr> assertions = smt.getAssertions();
	print (assertions);
}
int mine = 0;
void LiftedCVC4Problem::print(vector <Expr> &expression){
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


//void LiftedCVC4Problem::push(){
//	smt.push();
//}
//
//void LiftedCVC4Problem::pop(){
//	smt.pop();
//}

void LiftedCVC4Problem::insertAssertion(const Expr &e){
	if (!e.isNull()) {
		assertions.push_back(e);
	}
}

Expr LiftedCVC4Problem::andAssertionList (unsigned int begin, unsigned int end){
	if (end - begin <= em.maxArity(kind::AND)){
		return em.mkExpr(kind::AND, vector <Expr> (assertions.begin() + begin, assertions.begin() + end));
	}
	unsigned int middle = (begin + end) / 2;
	return em.mkExpr(kind::AND, andAssertionList(begin, middle), andAssertionList(middle, end));
}


Expr LiftedCVC4Problem::getAssertions(){
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

void LiftedCVC4Problem::clearAssertionList(){
	assertions.clear();
}

LiftedCVC4Problem::~LiftedCVC4Problem(){
}


//find and return the index of corresponding PVariableExpression in the variableExpr array
int LiftedCVC4Problem::getVariableIndex (int variableId, int operatorId, int significantTimePoint){
	if (operatorId == myProblem.operators.size() || myProblem.variables[variableId].ids[operatorId] == -1){
		significantTimePoint++;
		operatorId = 0;
	}

	int ret = significantTimePoint * nVariables + myProblem.variables[variableId].ids[operatorId];
	if (variableExpr[ret].isNull()){
		Type real = em.realType();
//		ostringstream oss;
//		oss << "[";
//		myProblem.variables[variableStateId].write(oss);
//		oss << ", " << significantTimePoint << "]";
//		variableExpr[ret] = Expr(em.mkVar(oss.str(),real));
		variableExpr[ret] = Expr(em.mkVar(real));
	}
	return ret;
}

//find and return the index of corresponding proposition in the propositionExpr array
int LiftedCVC4Problem::getPropositionIndex (int propositionId, int operatorId, int significantTimePoint){
	if (operatorId == myProblem.operators.size() || myProblem.propositions[propositionId].ids[operatorId] == -1){
		significantTimePoint++;
		operatorId = 0;
	}

	int ret = significantTimePoint * nPropositions + myProblem.propositions[propositionId].ids[operatorId];
	if (propositionExpr[ret].isNull()){
		Type boolean = em.booleanType();
//		ostringstream oss;
//		oss << "[";
//		myProblem.propositions[proposition].write(oss);
//		oss << ", " << significantTimePoint << "]";
//		propositionExpr[ret] = Expr(em.mkVar(oss.str(), boolean));
		propositionExpr[ret] = Expr(em.mkVar(boolean));
	}
	return ret;
}

//find and return the index of corresponding action in the actionExpr array
int LiftedCVC4Problem::getPartialActionIndex (MyPartialAction *partialAction, int significantTimePoint){
	int ret = significantTimePoint * nPartialActions + partialAction->id;
	if (partialActionExpr[ret].isNull()){
		vector <Expr> unifications;
		map <string, int>::iterator it, itEnd;
		it = partialAction->unificationId.begin();
		itEnd = partialAction->unificationId.end();
		for (; it != itEnd; ++it){
			unifications.push_back(unificationExpr[getUnificationIndex(it->second + partialAction->op->offset[partialAction->partialOperator->placement[it->first]], significantTimePoint)]);
		}
		partialActionExpr[ret] = em.mkExpr(kind::AND, unifications);
	}
	return ret;
}


//find and return the index of corresponding action in the actionExpr array
int LiftedCVC4Problem::getUnificationIndex (int unificationId, int significantTimePoint){
	int ret = significantTimePoint * nPartialActions + unificationId;
	if (unificationExpr[ret].isNull()){
		Type boolean = em.booleanType();
//		ostringstream oss;
//		oss << "[";
//		myProblem.actions[action].write(oss);
//		oss << ", " << significantTimePoint << "]";
//		actionExpr[ret] = Expr (em.mkVar (oss.str(), boolean));
		unificationId[ret] = Expr (em.mkVar (boolean));
	}
	return ret;
}





