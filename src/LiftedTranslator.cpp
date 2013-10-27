/*
 * LiftedTranslator.cpp
 *
 *  Created on: Sep 28, 2013
 *      Author: sadra
 */

#include "LiftedTranslator.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"
#include "Utilities.h"
#include "MyLiftedProposition.h"
#include "MyProblem.h"
#include "MyComparison.h"
#include "MyAssignment.h"

#include "VALfiles/FastEnvironment.h"
#include <list>

using namespace std;
using namespace Inst;
using namespace VAL;


namespace mdbr {

LiftedTranslator::LiftedTranslator() {

}

void LiftedTranslator::addInitialState(){
	map <Literal *, MyLiftedProposition>::iterator it, itEnd;
	it = myProblem.liftedPropositions.begin();
	itEnd = myProblem.liftedPropositions.end();
	for (; it != itEnd; ++it){
		int id, significantTimePoint;
		significantTimePoint = 0;
		findIdOfProposition(it->second, 0, id, significantTimePoint);
		polarity plrty = E_NEG;
		if (it->second.initialValue){
			plrty = E_POS;
		}
		solver.addProposition(plrty, id, significantTimePoint);
		solver.endClause();
	}

	if (myProblem.nValues > 0){
		int nVariables = myProblem.variables.size();
		for (int i = 0; i < nVariables; ++i){
			map <double, MyValue>::iterator valueIt, valueItEnd;
			valueIt = myProblem.variables[i].domain.begin();
			valueItEnd = myProblem.variables[i].domain.end();
			for (; valueIt !=  valueItEnd; ++valueIt){
				polarity plrty = E_NEG;
				if (valueIt->first == myProblem.variables[i].initialValue->value){
					plrty = E_POS;
				}
				int id, stp;
				stp = 0;
				findIdOfValue(valueIt->second, 0, id, stp);
				solver.addValue(plrty, id, stp);
				solver.endClause();
			}
		}
	}
}

void LiftedTranslator::findIdOfProposition (MyLiftedProposition &theProposition, int operatorIndex, int &id, int &significantTimePoint){
	if (operatorIndex == myProblem.operators.size()){
		operatorIndex = 0;
		significantTimePoint++;
	}
	if (theProposition.ids[operatorIndex] == -1){
		if (theProposition.ids[0] == -1){
			significantTimePoint = 0;
			id = theProposition.originalLiteral->getGlobalID();
			return;
		}
		id = theProposition.ids[0];
		significantTimePoint++;
		return;
	}
	id = theProposition.ids[operatorIndex];
	return;
}

void LiftedTranslator::findIdOfValue (MyValue &theValue, int operatorIndex, int &id, int &significantTimePoint){
	if (operatorIndex == myProblem.operators.size()){
		operatorIndex = 0;
		significantTimePoint++;
	}
	if (theValue.ids[operatorIndex] == -1){
		id = theValue.ids[0];
		significantTimePoint++;
		return;
	}
	id = theValue.ids[operatorIndex];
	return;
}


void LiftedTranslator::addGoal (const goal *gl, int operatorIndex, int significantTimePoint){
	const simple_goal *simpleGoal = dynamic_cast <const simple_goal *> (gl);
	if (simpleGoal){
		FastEnvironment env(0);
		Literal lit (simpleGoal->getProp(), &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		int id;
		findIdOfProposition(myProblem.liftedPropositions[lit2], operatorIndex, id, significantTimePoint);
		solver.addProposition(VAL::E_POS, id, significantTimePoint);
		solver.endClause();
		return;
	}
	const conj_goal *conjunctiveGoal = dynamic_cast <const conj_goal *> (gl);
	if (conjunctiveGoal){
		const goal_list *goalList = conjunctiveGoal->getGoals();
		goal_list::const_iterator it = goalList->begin();
		goal_list::const_iterator itEnd = goalList->end();
		for (; it != itEnd; it++){
			addGoal(*it, operatorIndex, significantTimePoint);
		}
		return;
	}
	CANT_HANDLE("WE DON'T SUPPORT SOME GOAL FOR THIS PROBLEM!!!");
	exit(1);
}

void LiftedTranslator::addGoals(int significantTimePoint){
	addGoal (current_analysis->the_problem->the_goal, 0, significantTimePoint);
}

void LiftedTranslator::addOperators(int significantTimePoint){
	int nOperators = myProblem.operators.size();
	for (int i = 0; i < nOperators; i++){
		int nArguments = myProblem.operators[i]->argument.size();
		for (int j = 0; j < nArguments; j++){

			//If an operator is used, at least one object should assign to its argument!
			solver.addOperators(VAL::E_NEG, i, significantTimePoint);
			int nUnification = myProblem.operators[i]->argument[j]->objects.size();
			for (int k = 0; k < nUnification; k++){
				solver.addUnification(VAL::E_POS, myProblem.operators[i]->offset[j] + k, significantTimePoint);
			}
			solver.endClause();


			//There should exist no two unification for one argument!
			for (int k = 0; k < nUnification; k++){
				for (int l = 0; l < nUnification; l++){
					if (k != l){
						solver.addUnification(VAL::E_NEG, myProblem.operators[i]->offset[j] + k, significantTimePoint);
						solver.addUnification(VAL::E_NEG, myProblem.operators[i]->offset[j] + l, significantTimePoint);
						solver.endClause();
					}
				}
			}
		}
	}
}

void LiftedTranslator::addPartialActions(int significantTimePoint){
	list <MyPartialAction>::iterator it, itEnd;
	it = myProblem.partialAction.begin();
	itEnd = myProblem.partialAction.end();

	for (; it != itEnd; ++it){
		//If a partial action be true then its corresponding proposition should also be true
		polarity plrty = VAL::E_POS;
		int stp = significantTimePoint;
		int id;
		if (it->pKind == mdbr::precondition){
			findIdOfProposition(*(it->proposition), it->op->id, id, stp);
		}else{
			findIdOfProposition(*(it->proposition), it->op->id + 1, id, stp);
		}
		if (it->pKind == mdbr::deleteEffect){
			plrty = VAL::E_NEG;
		}

		solver.addPartialAction(VAL::E_NEG, it->id, significantTimePoint);
		solver.addProposition(plrty, id, stp);
		solver.endClause();



		//If a partial action be true then its corresponding operator and objects should also be true
		solver.addPartialAction(VAL::E_NEG, it->id, significantTimePoint);
		solver.addOperators(VAL::E_POS, it->op->id, significantTimePoint);
		solver.endClause();
		int nArguments = it->objectId.size();

		for (int i = 0; i < nArguments; ++i){
			solver.addPartialAction(VAL::E_NEG, it->id, significantTimePoint);
			solver.addUnification(VAL::E_POS, it->objectId[i] + it->op->offset[it->liftedPartialAction->placement[i]], significantTimePoint);
			solver.endClause();
		}

		//If an operator and some of it's unifications are true then the corresponding partial action also should be true;
		solver.addOperators(VAL::E_NEG, it->op->id, significantTimePoint);
		for (int i = 0; i < nArguments; ++i){
			solver.addUnification(VAL::E_NEG, it->objectId[i] + it->op->offset[it->liftedPartialAction->placement[i]], significantTimePoint);
		}
		solver.addPartialAction(VAL::E_POS, it->id, significantTimePoint);
		solver.endClause();
	}

}

void LiftedTranslator::addComparisons(int significantTimePoint){
	list <MyComparison>::iterator it, itEnd;
	it = myProblem.comparisons.begin();
	itEnd = myProblem.comparisons.end();

	for (; it != itEnd; ++it){
		//If a comparison should hold then all of possible values that contradict with the comparison should not hold

		list < pair < list < MyValue* >, bool> >::iterator it1, it1End;
		it1 = it->possibleValues.begin();
		it1End = it->possibleValues.end();

		for (; it1 != it1End; ++it1){
			if (it1->second == false){
				solver.addComparison(VAL::E_NEG, it->comparisonId, significantTimePoint);
				list <MyValue *>::iterator it2, it2End;
				it2 = it1->first.begin();
				it2End = it1->first.end();
				for (; it2 != it2End; ++it2){
					int id, stp;
					stp = significantTimePoint;
					findIdOfValue(**it2, it->op->id, id, stp);
					solver.addValue(VAL::E_NEG, id, stp);
				}
				solver.endClause();
			}
		}


		//If a comparison be true then its corresponding operator and objects should also be true
		solver.addComparison(VAL::E_NEG, it->comparisonId, significantTimePoint);
		solver.addOperators(VAL::E_POS, it->op->id, significantTimePoint);
		solver.endClause();

		map <string, int>::iterator it3, it3End;
		it3 = it->objectId.begin();
		it3End = it->objectId.end();
		for (; it3 != it3End; ++it3){
			solver.addComparison(VAL::E_NEG, it->comparisonId, significantTimePoint);
			solver.addUnification(VAL::E_POS, it3->second + it->op->offset[it->liftedComparison->placement[it3->first]], significantTimePoint);
			solver.endClause();
		}

		//If an operator and some of it's unifications are true then the corresponding comparison also should be true;
		solver.addOperators(VAL::E_NEG, it->op->id, significantTimePoint);
		it3 = it->objectId.begin();
		for (; it3 != it3End; ++it3){
			solver.addUnification(VAL::E_NEG, it3->second + it->op->offset[it->liftedComparison->placement[it3->first]], significantTimePoint);
		}
		solver.addComparison(VAL::E_POS, it->comparisonId, significantTimePoint);
		solver.endClause();
	}

}

void LiftedTranslator::addAssignments(int significantTimePoint){
	list <MyAssignment>::iterator it, itEnd;
	it = myProblem.assignments.begin();
	itEnd = myProblem.assignments.end();

	for (; it != itEnd; ++it){
		//If an assignment should hold then for each possible values of variables we insert a clause to produce the correct value for the assignee variable

		list < pair < list < MyValue* >, MyValue*> >::iterator it1, it1End;
		it1 = it->possibleValues.begin();
		it1End = it->possibleValues.end();

		for (; it1 != it1End; ++it1){
			solver.addAssignment(VAL::E_NEG, it->assignmentId, significantTimePoint);
			list <MyValue *>::iterator it2, it2End;
			it2 = it1->first.begin();
			it2End = it1->first.end();
			int id, stp;
			for (; it2 != it2End; ++it2){
				stp = significantTimePoint;
				findIdOfValue(**it2, it->op->id, id, stp);
				solver.addValue(VAL::E_NEG, id, stp);
			}
			stp = significantTimePoint;
			findIdOfValue(*(it1->second), it->op->id + 1, id, stp);
			solver.addValue(VAL::E_POS, id, stp);
			solver.endClause();
		}


		//If an assignment be true then its corresponding operator and objects should also be true
		solver.addAssignment(VAL::E_NEG, it->assignmentId, significantTimePoint);
		solver.addOperators(VAL::E_POS, it->op->id, significantTimePoint);
		solver.endClause();

		map <string, int>::iterator it3, it3End;
		it3 = it->objectId.begin();
		it3End = it->objectId.end();
		for (; it3 != it3End; ++it3){
			solver.addAssignment(VAL::E_NEG, it->assignmentId, significantTimePoint);
			solver.addUnification(VAL::E_POS, it3->second + it->op->offset[it->liftedAssignment->placement[it3->first]], significantTimePoint);
			solver.endClause();
		}

		//If an operator and some of it's unifications are true then the corresponding assignment also should be true;
		solver.addOperators(VAL::E_NEG, it->op->id, significantTimePoint);
		it3 = it->objectId.begin();
		for (; it3 != it3End; ++it3){
			solver.addUnification(VAL::E_NEG, it3->second + it->op->offset[it->liftedAssignment->placement[it3->first]], significantTimePoint);
		}
		solver.addAssignment(VAL::E_POS, it->assignmentId, significantTimePoint);
		solver.endClause();
	}

}


void LiftedTranslator::addExplanatoryAxioms(int significantTimePoint){
	if (significantTimePoint < 1){
		return;
	}
	map <Inst::Literal *, MyLiftedProposition>::iterator it, itEnd;
	it = myProblem.liftedPropositions.begin();
	itEnd = myProblem.liftedPropositions.end();

	int nOperator = myProblem.operators.size();

	for (; it != itEnd; ++it){
		vector < list <MyPartialAction *> > adder, deleter;

		//If a proposition be true then there should some reason for it
		list <MyPartialAction *>::iterator it1, it1End;
		it1 = it->second.adder.begin();
		it1End = it->second.adder.end();

		adder.resize(nOperator);

		for (; it1 != it1End; ++it1){
			adder[(*it1)->op->id].push_back(*it1);
		}

		for (int i = 0; i < nOperator; ++i){

			if (adder[i].size()){

			}
		}


		solver.endClause();
	}


	it = myProblem.liftedPropositions.begin();

	for (; it != itEnd; ++it){
		//If a proposition be false in a layer then there should some reason for it
		list <MyPartialAction *>::iterator it1, it1End;
		it1 = it->second.deleter.begin();
		it1End = it->second.deleter.end();

		solver.addProposition(E_POS, it->second.id, significantTimePoint);
		solver.addProposition(E_NEG, it->second.id, significantTimePoint - 1);

		for (; it1 != it1End; ++it1){
			solver.addPartialAction(E_POS, (*it1)->id, significantTimePoint - 1);
		}

		solver.endClause();
	}

	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		map <double, MyValue>::iterator valueIt, valueItEnd;
		valueIt = myProblem.variables[i].domain.begin();
		valueItEnd = myProblem.variables[i].domain.end();
		for (; valueIt != valueItEnd; ++valueIt){



			//If a value become true in a layer then it should have some reason!
			solver.addValue(E_POS, valueIt->second.globalValueId, significantTimePoint - 1);
			solver.addValue(E_NEG, valueIt->second.globalValueId, significantTimePoint);
			list <MyAssignment *>::iterator assignerIt, assignerItEnd;
			assignerIt = myProblem.variables[i].assigner.begin();
			assignerItEnd = myProblem.variables[i].assigner.end();
			for (; assignerIt != assignerItEnd; ++assignerIt){
				solver.addAssignment(E_POS, (*assignerIt)->assignmentId, significantTimePoint - 1);
			}
			solver.endClause();



			//If a value become false in a layer then it should have some reason!
			solver.addValue(E_NEG, valueIt->second.globalValueId, significantTimePoint - 1);
			solver.addValue(E_POS, valueIt->second.globalValueId, significantTimePoint);
			assignerIt = myProblem.variables[i].assigner.begin();
			for (; assignerIt != assignerItEnd; ++assignerIt){
				solver.addAssignment(E_POS, (*assignerIt)->assignmentId, significantTimePoint - 1);
			}
			solver.endClause();

		}

	}
	*/
}

void LiftedTranslator::addAtomMutex(int significantTimePoint){

	int nOperators = myProblem.operators.size();

	int nPropositions = myProblem.propositions.size();

	for (int i = 0; i < nPropositions; ++i){
		for (int j = 0; j < i; ++i){
			if (myProblem.propositions[i].isMutex(significantTimePoint, &(myProblem.propositions[j]))){
				int lastId1, lastId2, id1, id2, stp1, stp2, lastStp1, lastStp2;
				lastStp1 = lastId1 = -1;
				for (int k = 0; k < nOperators; ++k){
					findIdOfProposition(myProblem.liftedPropositions[myProblem.propositions[i].originalLiteral], k, id1, stp1);
					findIdOfProposition(myProblem.liftedPropositions[myProblem.propositions[j].originalLiteral], k, id2, stp2);
					if (id1 == lastId1 && stp1 == lastStp1 && id2 == lastId2 && stp2 == lastStp2){
						continue;
					}
					solver.addProposition(E_NEG, id1, stp1);
					solver.addProposition(E_NEG, id2, stp2);
					solver.endClause();
					lastId1 = id1;
					lastStp1 = stp1;
					lastStp2 = stp2;
					lastId2 = id2;
				}
			}
		}
	}

	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; ++i){
		map <double, MyValue>::iterator valueIt, valueIt2, valueItEnd;
		valueIt = myProblem.variables[i].domain.begin();
		valueItEnd = myProblem.variables[i].domain.end();
		int id1, id2, stp, lastId1, lastStp;
		lastStp = lastId1 = -1;
		for (; valueIt != valueItEnd; ++valueIt){
			for (int j = 0; j < nOperators; ++j){
				findIdOfValue(valueIt->second, j, id1, stp);
				if (lastId1 == id1 && lastStp == stp){
					continue;
				}
				valueIt2 = myProblem.variables[i].domain.begin();
				for (; valueIt2 != valueIt; ++valueIt2){
					findIdOfValue(valueIt2->second, j, id2, stp);
					solver.addValue(E_NEG, id1, stp);
					solver.addValue(E_NEG, id2, stp);
					solver.endClause();
				}
			}
		}
	}

}

void LiftedTranslator::buildFormula(int nSignificantTimePoints){
	nCompletedSignificantTimePoint = 0;
	if (nSignificantTimePoints <= 0){
		return;
	}
	if (nCompletedSignificantTimePoint == 0){
		solver.prepareTrueValue();
		addInitialState();
		nCompletedSignificantTimePoint++;
	}
	for (; nCompletedSignificantTimePoint < nSignificantTimePoints; nCompletedSignificantTimePoint++){
		addAtomMutex(nCompletedSignificantTimePoint);
		addExplanatoryAxioms(nCompletedSignificantTimePoint);
		addOperators(nCompletedSignificantTimePoint - 1);
		addPartialActions(nCompletedSignificantTimePoint - 1);
		addComparisons(nCompletedSignificantTimePoint - 1);
		addAssignments(nCompletedSignificantTimePoint - 1);
	}

}

bool LiftedTranslator::solve (int nSignificantTimePoints){
	solver.refreshLGL();
	buildFormula(nSignificantTimePoints);

	addGoals(nSignificantTimePoints - 1);


	return solver.solving();
}


void LiftedTranslator::printSolution(ostream &sout){
	vector <MyOperator *>::iterator opIt, opItEnd;
	opItEnd = myProblem.operators.end();
	for (int i = 0; i < nCompletedSignificantTimePoint; i++){
		sout << ";;LEVEL: " << i << endl;
		opIt = myProblem.operators.begin();
		for (; opIt != opItEnd; ++opIt){
			if (solver.isTrueOperator((*opIt)->id, i)){
				sout <<"("<< (*opIt)->originalOperator->name->getName();
				int nArguments = (*opIt)->argument.size();
				for (int j = 0; j < nArguments; ++j){
					int nObjects = (*opIt)->argument[j]->objects.size();
					for (int k = 0; k < nObjects; k++){
						if (solver.isTrueUnification((*opIt)->offset[j] + k, i)){
							sout << " " << (*opIt)->argument[j]->objects[k]->originalObject->getName();
						}
					}
				}
				sout << ")" << endl;
			}
		}
	}
}


void LiftedTranslator::getSolution(vector <pair <operator_ *, FastEnvironment> > &solution){
	vector <MyOperator *>::iterator opIt, opItEnd;
	solution.clear();
	opItEnd = myProblem.operators.end();
	for (int i = 0; i < nCompletedSignificantTimePoint; i++){
		opIt = myProblem.operators.begin();
		for (; opIt != opItEnd; ++opIt){
			if (solver.isTrueOperator((*opIt)->id, i)){
				int nArguments = (*opIt)->argument.size();
				pair <operator_ *, FastEnvironment> action ( (*opIt)->originalOperator, FastEnvironment(nArguments));
//				action.first = (*opIt)->originalOperator;
//				action.second = FastEnvironment(nArguments);
				var_symbol_list::iterator paramaterIt;
				paramaterIt = action.first->parameters->begin();
				for (int j = 0; j < nArguments; ++j, ++paramaterIt){
					int nObjects = (*opIt)->argument[j]->objects.size();
					for (int k = 0; k < nObjects; k++){
						if (solver.isTrueUnification((*opIt)->offset[j] + k, i)){
							action.second[*paramaterIt] = (*opIt)->argument[j]->objects[k]->originalObject;
							break;
						}
					}
				}
				solution.push_back(action);
			}
		}
	}
}



void LiftedTranslator::writeSATProposition(ostream &sout){

	int counter = 2;
	for (int ll = 0; ll < nCompletedSignificantTimePoint; ++ll){
		int nOperators = myProblem.operators.size();
		for (int i = 0; i < nOperators; ++i){
			sout << counter++ << ":" << solver.getIdOfOperator(myProblem.operators[i]->id, ll) << ':' << myProblem.operators[i]->originalOperator->name->getName() << endl;
		}
		for (int i = 0; i < nOperators; ++i){
			int nArgument = myProblem.operators[i]->offset.size();
			for (int j = 0; j < nArgument; ++j){
				int nObject = myProblem.operators[i]->argument[j]->objects.size();
				for (int k = 0; k < nObject; ++k){
					sout << counter++ << ":" << solver.getIdOfUnification(myProblem.operators[i]->offset[j] + k, ll) << ':' << myProblem.operators[i]->originalOperator->name->getName() << ' ' << j << ':' << myProblem.operators[i]->argument[j]->objects[k]->originalObject->getName() << endl;
				}
			}
		}
		list <MyPartialAction>::iterator pAIt, pAItEnd;
		pAIt = myProblem.partialAction.begin();
		pAItEnd = myProblem.partialAction.end();
		for (; pAIt != pAItEnd; ++pAIt){
			cout << counter++ << ":" << solver.getIdOfPartialAction(pAIt->id, ll) << ':';
			pAIt->write(cout);
		}

		//I think we don't need to it anymore; If you are not agree with me, then you should change a little it
//		map <Literal*, MyLiftedProposition>::iterator  lAIt, lAItEnd;
//		lAIt = myProblem.liftedPropositions.begin();
//		lAItEnd = myProblem.liftedPropositions.end();
//		for (; lAIt != lAItEnd; ++lAIt){
//			if (lAIt->second.id >= 0){
//				cout << counter++ << ":" << solver.getIdOfProposition(lAIt->second.id, ll) << ':'; lAIt->first->write(cout); cout << endl;
//			}
//		}
	}
}

LiftedTranslator::~LiftedTranslator() {
	// TODO Auto-generated destructor stub
}

} /* namespace mdbr */
