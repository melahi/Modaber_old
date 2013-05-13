
#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "SMTProblem.h"


class Translator {
private:
	SMTProblem *smtProblem;
public:
	Translator(SMTProblem *smtProblem);

	//Translate initial state of planning problem to SMT problem
	void addInitialState();

	//Insert actions' conditions for the specified time point in smt problem
	void addActions (int significantTimePoint);

	//Add goals to the smt problem
	void addGoals (int significantTimePoint);

	//Insert Explanatory Axioms which is needed for SMT problem
	void addExplanatoryAxioms (int significantTimePoint);

	//Inset action mutex to the SMT problem
	void addActionMutex (int significantTimePoint);


	virtual ~Translator();

private:

	inline void addPropositionalActionPreconditions (int actionID, int significantTimePoint);
	inline void addNumericalActionPreconditions (int actionID, int significantTimePoint);
	inline void addPropositionalActionEffects (int actionID, int significantTimePoint);
	inline void addNumericalActionEffects (int actionID, int significantTimePoint);

};

#endif /* TRANSLATOR_H_ */
