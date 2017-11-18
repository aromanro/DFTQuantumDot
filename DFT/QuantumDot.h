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

			const Vector3D<double> center = dftSolver.realSpaceCell.GetSize() / 2.;
	
			for (int i = 0; i < size; ++i)
			{
				const Vector3D<double> distanceVector(dftSolver.realSpaceCell.SamplePoints(i) - center);

				if (harmonic)
					V(i) = 2. * distanceVector * distanceVector;
				else
					V(i) = 4. * distanceVector.Length();
					
			}

			dftSolver.SetPotential(V);
		}

		Solver& dftSolver;
	};

}