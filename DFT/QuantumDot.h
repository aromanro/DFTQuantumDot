#pragma once

#include "DftSolver.h"

namespace DFT {

	template<class Solver> class QuantumDot
	{
	public:
		QuantumDot(Solver& solver, bool harmonic = true)
			: dftSolver(solver)
		{
			dftSolver.f = 2;

			const int size = dftSolver.realSpaceCell.Samples();

			Eigen::MatrixXcd V(size, 1);

			Vector3D<double> center = dftSolver.realSpaceCell.GetSize() / 2.;
	
			for (int i = 0; i < size; ++i)
			{
				Vector3D<double> distanceVector(dftSolver.realSpaceCell.SamplePoints(i) - center);

				if (harmonic)
				{
					double lngth2 = distanceVector * distanceVector;
					V(i) = 2. * lngth2;
				}
				else
				{
					double lngth = distanceVector.Length();
					V(i) = 4. * lngth;
				}
					
			}

			dftSolver.SetPotential(V);
		}

		Solver& dftSolver;
	};

}