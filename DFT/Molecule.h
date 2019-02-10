#pragma once


#include "DftSolver.h"
#include "VWNExcCor.h"

#include "Vector3D.h"

// this is added later to the project, just as an example on how to use the dft solver to compute a molecule
// it's pretty basic, no pseudopotential so only simple ones can be solved
// you can ignore this, it's not needed to understand the 'quantum dot' part

// if you follow the lectures and support files pointed on the blog, this corresponds to assignment 3, part 2
// 2 Density functional calculation of molecules


namespace DFT {

	class Numerics
	{
	public:
		static double Gaussian3D(double distance, double sigma) 
		{ 
			const double sigma2 = sigma * sigma;
			const double sq = sqrt(2. * M_PI * sigma2);
			const double sq3 = sq * sq * sq;

			return exp(-distance*distance / (2.*sigma2)) / sq3; 
		}
	};

	template<class Solver> class Molecule
	{
	public:
		Molecule(Solver& solver, const std::vector<Vector3D<double>>& pos, const std::vector<double>& Zvals, int f = 2, double vsigma = 0.25)
			: dftSolver(solver), positions(pos), Z(pos.size(), 1), sigma(vsigma)
		{
			dftSolver.f = f;

			for (int i = 0; i < Zvals.size() && i < pos.size(); ++i)
				Z[i] = Zvals[i];

			const int size = dftSolver.realSpaceCell.Samples();
			
			// structure factor
			StructureFactor = Eigen::VectorXcd::Zero(size);

			for (int i = 0; i < size; ++i)
			{
				for (int charge = 0; charge < positions.size(); ++charge)
					StructureFactor(i) += Z[charge] * std::exp(std::complex<double>(0, -1) * (dftSolver.reciprocalCell.LatticeVectors(i) * positions[charge]));
			}

			// nuclear charge density
			Eigen::VectorXcd g(size);


			Vector3D<double> center = dftSolver.realSpaceCell.GetSize() / 2.;
			for (int i = 0; i < size; ++i)
			{
				Vector3D<double> distanceVector(dftSolver.realSpaceCell.SamplePoints(i) - center);

				g(i) = Numerics::Gaussian3D(distanceVector.Length(), sigma);
			}

			TRACE(L"Gaussian normalization check: %f\n", g.sum().real()* dftSolver.realSpaceCell.SampleVolume());

			rg.resize(size);
			solver.fft.fwd(g.data(), rg.data(), dftSolver.realSpaceCell.GetSamples().X, dftSolver.realSpaceCell.GetSamples().Y, dftSolver.realSpaceCell.GetSamples().Z);

			for (int i = 0; i < size; ++i)
			{
				rg(i) *= StructureFactor(i);
				rg(i) /= size;
			}

			NuclearChargeDensity.resize(size);
			solver.fft.inv(rg.data(), NuclearChargeDensity.data(), dftSolver.realSpaceCell.GetSamples().X, dftSolver.realSpaceCell.GetSamples().Y, dftSolver.realSpaceCell.GetSamples().Z);

			// Vdual
			Vrd.resize(size);
			Vrd(0) = 0;
			for (int i = 1; i < size; ++i)
				Vrd(i) = -4. * M_PI / dftSolver.reciprocalCell.LatticeVectorsSquaredMagnitude(i);

			Vr = Vrd.cwiseProduct(StructureFactor);

			dftSolver.SetReciprocalPotential(Vr);
		}

		double EwaldEnergy()
		{
			// compute approximate Ewald energy
			Eigen::VectorXcd field = dftSolver.SolvePoissonToRealSpace(NuclearChargeDensity);

			Eigen::VectorXcd fieldReciprocal(dftSolver.realSpaceCell.Samples());
			Eigen::VectorXcd densityReciprocal(dftSolver.realSpaceCell.Samples());

			dftSolver.fft.fwd(field.data(), fieldReciprocal.data(), dftSolver.realSpaceCell.GetSamples().X, dftSolver.realSpaceCell.GetSamples().Y, dftSolver.realSpaceCell.GetSamples().Z);
			dftSolver.fft.fwd(NuclearChargeDensity.data(), densityReciprocal.data(), dftSolver.realSpaceCell.GetSamples().X, dftSolver.realSpaceCell.GetSamples().Y, dftSolver.realSpaceCell.GetSamples().Z);
		
			densityReciprocal *= dftSolver.realSpaceCell.SampleVolume();
			fieldReciprocal  /= dftSolver.realSpaceCell.Samples();
		
			const double numericalResult = (fieldReciprocal.adjoint() * densityReciprocal)(0).real()/2.;

			double Uself = 0;			
			for (auto z : Z) Uself += z * z / (2 * sqrt(M_PI) * sigma);
			
			return numericalResult - Uself;
		}

		Solver& dftSolver;

		std::vector<Vector3D<double>> positions;
		std::vector<double> Z;
		double sigma;

		Eigen::VectorXcd StructureFactor;

		// with gaussian
		Eigen::VectorXcd rg; // for a gaussian, use structure factors to make up the charge distribution for all of them
		Eigen::VectorXcd NuclearChargeDensity;

		// with delta 'function'
		Eigen::VectorXcd Vrd; // for one, use structure factors to get Vr for all nuclear charges
		Eigen::VectorXcd Vr;
	};



	// How to use (the H2 example from assignment):
	// with ChachiyoExchCor you will get different results than those from the assignment, the reason is the different exchange correlation 
	// if Vosko-Wilk-Nusair is used, the results are basically the same

	/*
	
	DFT::DftSolver<DFT::VWNExchCor> dftSolver(16, 16, 16, 64, 64, 64);
	DFT::Molecule<decltype(dftSolver)> molecule(dftSolver, { {0,0,0}, {1.5, 0, 0} }, { 1, 1 }, 2);

	double ewald = molecule.EwaldEnergy();
	
	CString str;
	str.Format(L"Ewald Energy: %f\n", ewald);
	AfxMessageBox(str);

	unsigned int Ns = 1;

	Eigen::MatrixXcd W = Eigen::MatrixXcd::Random(dftSolver.realSpaceCell.Samples(), Ns);

	W = dftSolver.orthogonalize(W);
	DFT::SolutionFinders::BaseClasses::SteepestDescent<decltype(dftSolver)> steepestDescent(dftSolver);
	W = steepestDescent.Descend(W, 20);
	W = dftSolver.orthogonalize(W);
	
	DFT::SolutionFinders::FletcherReevesConjugateGradient<decltype(dftSolver)> precgDescent(dftSolver);
	W = precgDescent.Descend(W, 30);

	double E = dftSolver.getE(W);

	str.Format(L"H2 electronic energy: %f", E);
	AfxMessageBox(str);

	str.Format(L"H2 total energy: %f", E + ewald);
	AfxMessageBox(str);

	auto ps = dftSolver.getPsi(W);

	str.Format(L"Energy level: %f", ps.second(0).real());
	AfxMessageBox(str);

	*/
}