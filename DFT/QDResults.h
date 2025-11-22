#pragma once

#undef min
#undef max
#include <eigen/eigen>

#include "Options.h"

#include "RealSpace.h"
#include "ReciprocalSpace.h"

class QDResults
{
public:
	QDResults();
	~QDResults();


	Options options;

	DFT::RealSpaceCell realSpaceCell;
	DFT::ReciprocalSpaceCell reciprocalCell;

	std::vector<double> maxs;
	double theMax;
	std::vector<double> energies;

	std::vector<Eigen::VectorXcd> wavefunctions;
	void FreeSomeMemory();

protected:
	template<class T> void FreeVector(std::vector<T>& v);
};

