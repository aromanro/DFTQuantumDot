#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <atomic>

#include "FFT.h"
#include "RealSpace.h"
#include "ReciprocalSpace.h"

#include <unsupported/Eigen/MatrixFunctions>


#include "ExcCor.h"

namespace DFT {

	// 'Derived' is used for 'curiously recurring template pattern' - see SolvePoissonToRealSpace, getgrad and getPsi implementations
	template<class ExchCor, class Derived> class DftSolverBase
	{
	public:
		DftSolverBase(double dim1, double dim2, double dim3, unsigned int samples1, unsigned int samples2, unsigned int samples3, unsigned int Ns)
			: realSpaceCell(dim1, dim2, dim3, samples1, samples2, samples3),
			reciprocalCell(realSpaceCell),
			f(1), terminate(false), m_Ns(Ns)
		{
		}

		inline void SetPotential(const Eigen::MatrixXcd& V)
		{
			dualV = cJdagOcJ(V);
		}


		inline void AddPotential(const Eigen::MatrixXcd& V)
		{
			dualV += cJdagOcJ(V);
		}
		
		inline void SetReciprocalPotential(const Eigen::MatrixXcd& V)
		{
			dualV = cJdag(V).real();
		}

		inline static double Dot(const Eigen::MatrixXcd& a, const Eigen::MatrixXcd& b)
		{
			return (a.adjoint() * b).trace().real();
		}

		inline Eigen::MatrixXcd SolvePoissonToReciprocalSpace(const Eigen::MatrixXcd& chargeDensity)
		{
			return -4. * M_PI * Linv(O(cJ(chargeDensity)));
		}

		inline Eigen::MatrixXcd SolvePoissonToRealSpace(const Eigen::MatrixXcd& chargeDensity)
		{
			return static_cast<Derived*>(this)->cI(SolvePoissonToReciprocalSpace(chargeDensity)).real();
		}


		inline Eigen::MatrixXcd cJ(const Eigen::MatrixXcd& in)
		{
			const Eigen::MatrixXcd out = InvFFT3D(in);

			const double norm = 1. / realSpaceCell.Samples();

			return norm * out;
		}


		inline Eigen::MatrixXcd cJdag(const Eigen::MatrixXcd& in)
		{
			const Eigen::MatrixXcd out = FFT3D(in);
		
			const double norm = 1. / realSpaceCell.Samples();
				
			return norm * out;
		}

		// overlap operator - it is diagonal for plane waves basis
		inline Eigen::MatrixXcd O(const Eigen::MatrixXcd& in) const
		{
			return realSpaceCell.Volume() * in;
		}


		inline Eigen::MatrixXcd cJdagOcJ(const Eigen::MatrixXcd& in)
		{
			return realSpaceCell.SampleVolume() * in;
		}

		inline Eigen::MatrixXcd orthogonalize(const Eigen::MatrixXcd& W) const
		{
			return W * (W.adjoint() * O(W)).sqrt().inverse();
		}


		// inverse Laplacian operator in reciprocal space
		inline Eigen::MatrixXcd Linv(const Eigen::MatrixXcd& in) const
		{
			Eigen::MatrixXcd out = -1. / realSpaceCell.Volume() * in.cwiseQuotient(reciprocalCell.LatticeVectorsSquaredMagnitude);

			out(0) = 0;

			return out;
		}

		inline Eigen::MatrixXcd diagouter(const Eigen::MatrixXcd& A, const Eigen::MatrixXcd& B) const
		{
			return A.cwiseProduct(B.conjugate()).rowwise().sum();			
		}

		inline Eigen::MatrixXcd Diagprod(const Eigen::MatrixXcd& a, const Eigen::MatrixXcd& B) const
		{
			return (a * Eigen::MatrixXcd::Ones(1, B.cols())).cwiseProduct(B);
		}

		inline Eigen::MatrixXcd getgrad(const Eigen::MatrixXcd& W)
		{
			const Eigen::MatrixXcd Wadj = W.adjoint();
			const Eigen::MatrixXcd OW = O(W);
			const Eigen::MatrixXcd Uinv = (Wadj * OW).inverse();
			const Eigen::MatrixXcd HW = static_cast<Derived*>(this)->H(W);
  
			return f * (HW - (OW * Uinv) * (Wadj * HW)) * Uinv;
		}


		inline Eigen::MatrixXcd getn(const Eigen::MatrixXcd& Psi)
		{
			Eigen::MatrixXcd IPsi = static_cast<Derived*>(this)->cI(Psi.col(0));
			Eigen::MatrixXcd n = IPsi.cwiseProduct(IPsi.conjugate());

			for (unsigned int col = 1; col < Psi.cols(); ++col)
			{
				IPsi = static_cast<Derived*>(this)->cI(Psi.col(col));
				n += IPsi.cwiseProduct(IPsi.conjugate());
			}

			return f * n;
		}


		inline std::pair<Eigen::MatrixXcd, Eigen::VectorXcd> getPsi(const Eigen::MatrixXcd& W)
		{
			const Eigen::MatrixXcd Y = orthogonalize(W);
			const Eigen::MatrixXcd mu = Y.adjoint() * static_cast<Derived*>(this)->H(Y); // <Yi|H|Yj> matrix

			eigensolver.compute(mu);
			
			assert(eigensolver.info() == Eigen::ComputationInfo::Success);

			const Eigen::MatrixXcd Psi = Y * eigensolver.eigenvectors();

			return std::make_pair(Psi, eigensolver.eigenvalues());
		}



		RealSpaceCell realSpaceCell;
		ReciprocalSpaceCell reciprocalCell;

		Fourier::FFT fft;

		unsigned int f; //occupancy, default 1, for the quantum dot 2

		std::atomic_bool terminate;
	
	protected:
		inline Eigen::MatrixXcd FFT3D(const Eigen::MatrixXcd& in)
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());
		
			for (unsigned int i = 0; i < in.cols(); ++i)
				fft.fwd(in.col(i).data(), out.col(i).data(), realSpaceCell.GetSamples().X, realSpaceCell.GetSamples().Y, realSpaceCell.GetSamples().Z);
				
			return out;
		}


		inline Eigen::MatrixXcd InvFFT3D(const Eigen::MatrixXcd& in)
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());
		
			for (unsigned int i = 0; i < in.cols(); ++i)
				fft.inv(in.col(i).data(), out.col(i).data(), realSpaceCell.GetSamples().X, realSpaceCell.GetSamples().Y, realSpaceCell.GetSamples().Z);
				
			return out;
		}


		unsigned int m_Ns;

		Eigen::MatrixXcd dualV;
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver;

		// to avoid reallocations and reinitializations
		ExchCor excCor;
	};

}