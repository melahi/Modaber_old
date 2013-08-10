/*
 * MutexFinder.h
 *
 *  Created on: May 14, 2013
 *      Author: sadra
 */

#ifndef MUTEXFINDER_H_
#define MUTEXFINDER_H_

#include <vector>
#include <ptree.h>
#include <algorithm>
#include "VALfiles/instantiation.h"
#include "VALfiles/FastEnvironment.h"
#include "Utilities.h"

using namespace std;
using namespace Inst;
using namespace VAL;





class MyAnalyzer{
public:

	vector < set <int> > mutexActions;
	vector < vector <int> > adderActions;
	vector < vector <int> > deleterActions;
	vector < vector <int> > variableModifierActions;


	MyAnalyzer(){
		adderActions.resize(instantiatedOp::howManyNonStaticLiterals());
		deleterActions.resize(instantiatedOp::howManyNonStaticLiterals());
		variableModifierActions.resize(instantiatedOp::howManyNonStaticPNEs());
		mutexActions.resize(instantiatedOp::howMany());
		findRelationOfActionsPropositions();
		findActionsMutexes();
	}

	bool isTwoActionMutex (int actionId1, int actionId2){
		if (actionId1 > actionId2){
			swap(actionId1, actionId2);
		}
		return (mutexActions[actionId1].find(actionId2) != mutexActions[actionId2].end());
	}

	void findRelationOfActionsPropositions(){
		OpStore::iterator iter, itEnd;
		iter = instantiatedOp::opsBegin();
		itEnd = instantiatedOp::opsEnd();
		for (;iter != itEnd; iter++){
			const operator_ *oper = (*iter)->forOp();
			FastEnvironment *env = (*iter)->getEnv();
			pc_list <simple_effect *>::const_iterator iter2, iterEnd2;



			//Initialize AdderActions vector
			iter2 = oper->effects->add_effects.begin();
			iterEnd2 = oper->effects->add_effects.end();
			for (;iter2 != iterEnd2; iter2++){
				Literal lit ((*iter2)->prop,env);
				const Literal *lit2 = instantiatedOp::findLiteral(&lit);
				adderActions [lit2->getStateID()].push_back((*iter)->getID());
			}


			//Initialize delterActions vector
			iter2 = oper->effects->del_effects.begin();
			iterEnd2 = oper->effects->del_effects.end();
			for (;iter2 != iterEnd2; iter2++){
				Literal lit ((*iter2)->prop,env);
				const Literal *lit2 = instantiatedOp::findLiteral(&lit);
				deleterActions [lit2->getStateID()].push_back((*iter)->getID());
			}

			//Initialize variableModifierAction vector
			pc_list <assignment *>::const_iterator asgnIter2, asgnIterEnd2;
			asgnIter2 = oper->effects->assign_effects.begin();
			asgnIterEnd2 = oper->effects->assign_effects.end();
			for (;asgnIter2 != asgnIterEnd2; asgnIter2++){
				PNE pne ((*asgnIter2)->getFTerm(),env);
				const PNE *pne2 = instantiatedOp::findPNE(&pne);
				variableModifierActions [pne2->getStateID()].push_back((*iter)->getID());
			}
		}
	}

	void insert2MutexActions(int actionId1, int actionId2){
		mutexActions[actionId1].insert(actionId2);
		mutexActions[actionId2].insert(actionId1);
	}


	class MutexFinder{
		MyAnalyzer *analyzer;
		FastEnvironment *env;
		int operatorId;

	public:

		MutexFinder (MyAnalyzer *analyzer, FastEnvironment *env, int operatorId):
			analyzer (analyzer), env(env), operatorId(operatorId){}

		void simpleGoalPreconditionMutexFinder(const proposition *prop){
			Literal lit = Literal(prop, env);
			Literal *lit2 = instantiatedOp::findLiteral(&lit);
			vector < int >:: const_iterator iter;
			vector < int >:: const_iterator iterEnd;
			if (lit2->getStateID() == -1){
				return;
			}
			iter = analyzer->deleterActions[lit2->getStateID()].begin();
			iterEnd = analyzer->deleterActions[lit2->getStateID()].end();
			for (;iter != iterEnd; ++iter){
				if ( *iter != operatorId){
					analyzer->insert2MutexActions(*iter, operatorId);
				}
			}
		}

		void expressionMutextFinder (const expression *expr){
			const binary_expression* binary = dynamic_cast <const binary_expression *> (expr);
			if (binary){
				expressionMutextFinder(binary->getLHS());
				expressionMutextFinder(binary->getRHS());
				return;
			}
			const uminus_expression* uMinus = dynamic_cast<const uminus_expression *> (expr);
			if (uMinus){
				expressionMutextFinder(uMinus->getExpr());
				return;
			}
			const num_expression* numExpr = dynamic_cast<const num_expression *> (expr);
			if (numExpr){
				return;
			}
			const func_term* functionTerm = dynamic_cast<const func_term *> (expr);
			if (functionTerm){
				PNE pne = PNE(functionTerm, env);
				PNE *pne2 = instantiatedOp::findPNE(&pne);
				if (pne2->getStateID() == -1){
					return;
				}
				vector < int >:: const_iterator iter;
				vector < int >:: const_iterator iterEnd;
				iter = analyzer->variableModifierActions[pne2->getStateID()].begin();
				iterEnd = analyzer->variableModifierActions[pne2->getStateID()].end();
				for (;iter != iterEnd; ++iter){
					if (operatorId != *iter){
						analyzer->insert2MutexActions(operatorId, *iter);
					}
				}

				return;
			}
			CANT_HANDLE("Can't handle some expression in analyzing!")
			return;
		}


		void operator() (const goal *gl){
			const simple_goal *simple = dynamic_cast<const simple_goal *>(gl);
			if (simple){
				simpleGoalPreconditionMutexFinder(simple->getProp());
				return;
			}
			const comparison *comp = dynamic_cast<const comparison*> (gl);
			if (comp){
				expressionMutextFinder(comp->getLHS());
				expressionMutextFinder(comp->getRHS());
				return;
			}
			const conj_goal *conjunctive = dynamic_cast<const conj_goal *>(gl);
			if (conjunctive){
				const goal_list *goalList = conjunctive->getGoals();
				for_each(goalList->begin(), goalList->end(), *this);
				return;
			}
			CANT_HANDLE("Can't handle some precondition in analyzing!")
			return;
		}

	};

	void findActionsMutexes(){

		//Action which adds some proposition is mutex with action which deletes it
		int nPropositions = deleterActions.size();
		for (int i = 0; i < nPropositions; i++){
			int numberOfAdderActions = adderActions[i].size();
			int numberOfDeleterActions = deleterActions[i].size();
			for (int j1 = 0; j1 < numberOfAdderActions; j1++){
				for (int j2 = 0; j2 < numberOfDeleterActions; j2++){
					insert2MutexActions(adderActions[i][j1], deleterActions[i][j2]);
				}
			}
		}

		//For now, we don't consider effect of more than one action on a single variable, so we consider two actions are mutex if they affect on a single variable
		int nVariable = variableModifierActions.size();
		for (int i = 0; i < nVariable; i++){
			int nMutexes = variableModifierActions[i].size();
			for (int j1 = 0; j1 < nMutexes; j1++){
				for (int j2 = j1 + 1; j2 < nMutexes; j2++){
					insert2MutexActions(variableModifierActions[i][j1], variableModifierActions[i][j2]);
				}
			}
		}

		//Action which needs some precondition is mutex with action which modifies (delete or change the value of) it
		OpStore::iterator iter, itEnd;
		iter = instantiatedOp::opsBegin();
		itEnd = instantiatedOp::opsEnd();
		for (;iter != itEnd; iter++){
			const operator_ *oper = (*iter)->forOp();
			FastEnvironment *env = (*iter)->getEnv();
			MutexFinder myMutexFinder(this, env, (*iter)->getID());
			myMutexFinder(oper->precondition);
		}

		return;
	}

	void printAnalysis(){
		int nOperator = mutexActions.size();
		for (int i = 0; i < nOperator; i++){
			set <int>::const_iterator iter, iterEnd;
			iter = mutexActions[i].begin();
			iterEnd = mutexActions[i].end();
			cout << "Action : " << i << endl << '\t';
			for (;iter != iterEnd; iter++){
				cout << *iter << ' ';
			}
			cout << endl;
		}
		int nProposition = adderActions.size();
		for (int i = 0; i < nProposition; i++){
			int nAction = adderActions[i].size();
			cout << "Adder proposition: " << i << endl << '\t';
			for (int j = 0; j < nAction; j++){
				cout << adderActions[i][j] << ' ';
			}
			cout << endl;
			nAction = deleterActions[i].size();
			cout << "deleter proposition: " << i << endl << '\t';
			for (int j = 0; j < nAction; j++){
				cout << deleterActions[i][j] << ' ';
			}
			cout << endl;
		}
		int nVariable = variableModifierActions.size();
		for (int i = 0; i < nVariable; i++){
			int nAction = variableModifierActions[i].size();
			cout << "modifier variable: " << i << endl << '\t';
			for (int j = 0; j < nAction; j++){
				cout << variableModifierActions[i][j] << ' ';
			}
			cout << endl;
		}
	}
};


#endif /* MUTEXFINDER_H_ */
