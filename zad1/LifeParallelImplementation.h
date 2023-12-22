/*
 * LifeParallelImplementation.h
 */

#ifndef LIFEPARALLELIMPLEMENTATION_H_
#define LIFEPARALLELIMPLEMENTATION_H_

#include "Life.h"

class LifeParallelImplementation: public Life {
protected:
	void realStep();
public:
	LifeParallelImplementation();
	int numberOfLivingCells();
	double averagePollution();
	void oneStep();
	void sequentionalStep();
	void beforeFirstStep();
	void afterLastStep();
	void giveWorkingRange(int rank, int procs, int &start, int &end);
};

#endif /* LIFEPARALLELIMPLEMENTATION_H_ */
