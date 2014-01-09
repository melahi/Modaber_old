

#include "LiftedCVC4Problem.h"
#include "VALfiles/parsing/ptree.h"


#include "sstream"
#include <limits>


using namespace VAL;
using namespace std;

#define PRINT_FORMULA


class ExpressionConvertor {
public:
	FastEnvironment *env;
	LiftedCVC4Problem *liftedCVC4Problem;
	int operatorId;
	int significantTimepoint;
	ExpressionConvertor (FastEnvironment *env, LiftedCVC4Problem *cvc4Problem, int operatorId, int significantTime): env(env), liftedCVC4Problem(cvc4Problem), operatorId(operatorId), significantTimepoint(significantTime){};

	Expr convertExpressionToCVC4Expr (const expression* expr){

		//Binary expression
		const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
		if (binary){
			Kind operatorKind;
			if (dynamic_cast <const plus_expression* > (expr)){
				operatorKind = kind::PLUS;
			}else if (dynamic_cast<const minus_expression *> (expr)){
				operatorKind = kind::MINUS;
			}else if (dynamic_cast<const mul_expression *> (expr)) {
				operatorKind = kind::MULT;
			}else if (dynamic_cast<const div_expression *> (expr)){
				operatorKind = kind::DIVISION;
			}else{
				operatorKind = kind::UNDEFINED_KIND;
				CANT_HANDLE("binary_expression");
			}
			Expr left = convertExpressionToCVC4Expr(binary->getLHS());
			Expr right = convertExpressionToCVC4Expr(binary->getRHS());
			return liftedCVC4Problem->em.mkExpr(operatorKind, left, right);
		}

		//Unary Minus
		const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
		if (uMinus){
			Expr uMinusExpr = convertExpressionToCVC4Expr(uMinus->getExpr());
			return liftedCVC4Problem->em.mkExpr(kind::UMINUS, uMinusExpr);
		}

		//Constant
		const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
		if (numExpr){
			long double myDouble = numExpr->double_value();
			int nominator, denominator;
			simpleConvertToRational(myDouble, nominator, denominator);
			return liftedCVC4Problem->em.mkConst(Rational(nominator, denominator));
		}

		//Variable
		const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
		if (functionTerm){
			PNE pne = PNE(functionTerm, env);
			PNE *pne2 = instantiatedOp::findPNE(&pne);
			if (pne2->getStateID() == -1){
				double myDouble = myProblem.initialValue[pne2->getGlobalID()];
				int nominator, denominator;
				simpleConvertToRational(myDouble, nominator, denominator);
				return liftedCVC4Problem->em.mkConst(Rational(nominator, denominator));
			}
			int index = liftedCVC4Problem->getVariableIndex(pne2->getStateID(), operatorId, significantTimepoint);
			return liftedCVC4Problem->variableExpr[index];
		}
		const violation_term *preferenceViolation = dynamic_cast <const violation_term *> (expr);
		if (preferenceViolation){
			return liftedCVC4Problem->getPreferenceExpr(preferenceViolation->getName());
		}
		CANT_HANDLE("can't handle One expression in converting to CVC4EXPR");
		return Expr();
	}

	static void simpleConvertToRational (double input, int &nominator, int &denominator){
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
#ifdef PRINT_FORMULA
	smt.setOption("interactive-mode", SExpr("true"));		//In the case of debugging we can turn it to "true"
#else
	smt.setOption("interactive-mode", SExpr("false"));		//In the case of debugging we can turn it to "true"
#endif
	smt.setOption("produce-assignments", SExpr("true"));
	smt.setOption("verbosity", SExpr("1073741823"));
//	smt.setOption("verbosity", SExpr("0"));
	smt.setOption("incremental", SExpr("true"));
	smt.setLogic("QF_LIRA");

	maximumSignificantTimePoint = 0;

	guaranteeSize(1);


	Type boolean = em.booleanType();
#ifdef PRINT_FORMULA
	ostringstream oss;
	oss << "[ TRUE ]";
	trueExpr = Expr(em.mkVar(oss.str(), boolean));
#else
	trueExpr = Expr(em.mkVar(boolean));
#endif
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
	addConditionToCluase(myProposition->originalLiteral->getStateID(), operatorId, significantTimePoint, polarity);
}




void LiftedCVC4Problem::addPartialActionToClause (MyPartialAction *partialAction, int significantTimePoint, bool polarity){
	int index = getPartialActionIndex(partialAction, significantTimePoint);
	if (polarity){
		buildingClause.push_back(partialActionExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, partialActionExpr[index]));
	}
}


void LiftedCVC4Problem::addUnificationToClause(int unificationId, int significantTimePoint, bool polarity){
	int index = getUnificationIndex(unificationId, significantTimePoint);
	if (polarity){
		buildingClause.push_back(unificationExpr[index]);
	}else{
		buildingClause.push_back(em.mkExpr(kind::NOT, unificationExpr[index]));
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


void LiftedCVC4Problem::AddConditionToCluase(const expression* leftExpresion, FastEnvironment *env, int operatorId, comparison_op compOp, double rightValue, int significantTimePoint){
	ExpressionConvertor myConvertor(env, this, operatorId, significantTimePoint);
	Expr left = myConvertor.convertExpressionToCVC4Expr(leftExpresion);
	float_expression rightExpression (rightValue);
	Expr right = myConvertor.convertExpressionToCVC4Expr(&rightExpression);
	Kind operatorKind;
	switch(compOp){
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
	ExpressionConvertor variableConvertor(env, this, operatorId, significantTimePoint);
	Expr variable = variableConvertor.convertExpressionToCVC4Expr(numericalAssignment->getFTerm());
	if (variable.isConst()){
		//This case is happening just in initial case when initial state determine the value of a static variable
		return;
	}
	ExpressionConvertor expressionConvertor(env, this, operatorId - 1, significantTimePoint);
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

void LiftedCVC4Problem::AddEqualityCondition (Expr expr, double value){

	int nominator, denominator;
	ExpressionConvertor::simpleConvertToRational(value, nominator, denominator);
	Expr valExpr = em.mkConst(Rational(nominator, denominator));
	Expr myCondtion = em.mkExpr(kind::EQUAL, expr, valExpr);
	buildingClause.push_back(myCondtion);
}

double LiftedCVC4Problem::getExpressionValue (const expression *valExpr, FastEnvironment *env, int operatorId, int significantTimePoint){
	ExpressionConvertor myConvertor(env, this, operatorId, significantTimePoint);
	Expr cvc4Expr = myConvertor.convertExpressionToCVC4Expr(valExpr);
	Rational rational = smt.getValue(cvc4Expr).getConst<Rational>();
	return rational.getDouble();
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
void LiftedCVC4Problem::print(vector <Expr> &expression){
	for (size_t i = 0; i < expression.size(); i++){
		if (expression[i].getKind() == kind::AND){
			vector <Expr> child = expression[i].getChildren();
			print(child);
		}else{
			expression[i].toStream(cout);
			cout << endl;
		}
	}
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

bool LiftedCVC4Problem::isUnificationUsed (int unificationId, int significantTimePoint){
	int unificationIndex = getUnificationIndex(unificationId, significantTimePoint);
	bool isUsed = smt.getValue(unificationExpr[unificationIndex]).getConst<bool>();
	return isUsed;
}

bool LiftedCVC4Problem::isPartialActionUsed(MyPartialAction *partialAction, int significantTimePoint){
	int partialActionIndex = getPartialActionIndex(partialAction, significantTimePoint);
	bool isUsed = smt.getValue(partialActionExpr[partialActionIndex]).getConst<bool>();
	return isUsed;
}

LiftedCVC4Problem::~LiftedCVC4Problem(){
}


//find and return the index of corresponding PVariableExpression in the variableExpr array
int LiftedCVC4Problem::getVariableIndex (unsigned int variableId, unsigned int operatorId, int significantTimePoint){
	if (operatorId == myProblem.operators.size() || myProblem.variables[variableId].ids[operatorId] == -1){
		significantTimePoint++;
		operatorId = 0;
	}

	int ret = significantTimePoint * nVariables + myProblem.variables[variableId].ids[operatorId];
	if (variableExpr[ret].isNull()){
		Type real = em.realType();
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[";
		myProblem.variables[variableId].originalPNE->write(oss);
		oss << ", " << significantTimePoint << ":" << operatorId << "]";
		variableExpr[ret] = Expr(em.mkVar(oss.str(),real));
#else
		variableExpr[ret] = Expr(em.mkVar(real));
#endif
	}
	return ret;
}

Expr LiftedCVC4Problem::getPreferenceExpr (string name){
	map <string, Expr>::iterator it;
	it = preferenceExpr.find(name);
	if (it == preferenceExpr.end()){
		Type real = em.realType();
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[ preference: " << name << " ]";
		preferenceExpr[name] = Expr(em.mkVar(oss.str(),real));
#else
		preferenceExpr[name] = Expr(em.mkVar(real));
#endif
		it = preferenceExpr.find(name);
	}
	return it->second;
}


//find and return the index of corresponding proposition in the propositionExpr array
int LiftedCVC4Problem::getPropositionIndex (unsigned int propositionId, unsigned int operatorId, int significantTimePoint){
	if (operatorId == myProblem.operators.size() || myProblem.propositions[propositionId].ids[operatorId] == -1){
		significantTimePoint++;
		operatorId = 0;
	}

	int ret = significantTimePoint * nPropositions + myProblem.propositions[propositionId].ids[operatorId];
	if (propositionExpr[ret].isNull()){
		Type boolean = em.booleanType();
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[";
		myProblem.propositions[propositionId].write(oss);
		oss << ", " << significantTimePoint << ":" << operatorId << "]";
		propositionExpr[ret] = Expr(em.mkVar(oss.str(), boolean));
#else
		propositionExpr[ret] = Expr(em.mkVar(boolean));
#endif
	}
	return ret;
}

//find and return the index of corresponding action in the actionExpr array
int LiftedCVC4Problem::getPartialActionIndex (MyPartialAction *partialAction, int significantTimePoint){
	int ret = significantTimePoint * nPartialActions + partialAction->id;
	if (partialActionExpr[ret].isNull()){
		Type boolean = em.booleanType();
		map <string, int>::iterator it, itEnd;
		it = partialAction->unificationId.begin();
		itEnd = partialAction->unificationId.end();
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[";
		oss << "(" << partialAction->op->originalOperator->name->getName();
		for (; it != itEnd; ++it){
			int unificationIndex = it->second + partialAction->op->offset[partialAction->partialOperator->placement[it->first]];
			oss << ' ' << partialAction->partialOperator->argument[it->first]->objects[it->second]->originalObject->getName() << ":" << unificationIndex;
		}
		oss << ")";
		oss << ", " << significantTimePoint << "]";
		partialActionExpr[ret] = Expr(em.mkVar(oss.str(), boolean));
#else
		partialActionExpr[ret] = Expr(em.mkVar(boolean));
#endif

	}
	return ret;
}


//find and return the index of corresponding action in the actionExpr array
int LiftedCVC4Problem::getUnificationIndex (int unificationId, int significantTimePoint){
	int ret = significantTimePoint * nUnifications + unificationId;
	if (unificationExpr[ret].isNull()){
		Type boolean = em.booleanType();
#ifdef PRINT_FORMULA
		ostringstream oss;
		oss << "[" << unificationId << ", " << significantTimePoint << "]";
		unificationExpr[ret] = Expr (em.mkVar (oss.str(), boolean));
#else
		unificationExpr[ret] = Expr (em.mkVar (boolean));
#endif
	}
	return ret;
}





