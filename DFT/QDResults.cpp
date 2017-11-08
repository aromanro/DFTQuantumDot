#include "stdafx.h"
#include "QDResults.h"


QDResults::QDResults()
	: realSpaceCell(6, 6, 6, 32, 32, 32), reciprocalCell(realSpaceCell),
	maxs(10), theMax(0)
{
}


QDResults::~QDResults()
{
}


void QDResults::FreeSomeMemory()
{
	wavefunctions.resize(0);
	realSpaceCell.Indices.resize(0);
	realSpaceCell.SamplePoints.resize(0);

	reciprocalCell.Indices.resize(0);
	reciprocalCell.LatticeVectors.resize(0);
	reciprocalCell.LatticeVectorsSquaredMagnitude.resize(0);
}
