#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


#include "FFT.h"
#include "RealSpace.h"
#include "ReciprocalSpace.h"

#include <unsupported/Eigen/MatrixFunctions>


#include "ExcCor.h"

namespace DFT {


	template<class ExchCor> class DftSolver
	{
	public:
		DftSolver(double dim1, double dim2, double dim3, unsigned int samples1, unsigned int samples2, unsigned int samples3)
			: realSpaceCell(dim1, dim2, dim3, samples1, samples2, samples3),
			reciprocalCell(realSpaceCell),
			f(1)
		{
		}


		inline void SetPotential(const Eigen::MatrixXcd& V)
		{
			dualV = cJdagOcJ(V);
		}

		inline void SetReciprocalPotential(const Eigen::MatrixXcd& V)
		{
			dualV = cJdag(V).real();
		}

		template<typename Derived> inline static double Dot(const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b)
		{
			return (a.adjoint() * b).trace().real();
		}

		template<typename Derived> inline Eigen::MatrixXcd SolvePoissonToReciprocalSpace(const Eigen::MatrixBase<Derived>& chargeDensity)
		{
			return -4. * M_PI * Linv(O(cJ(chargeDensity)));
		}

		template<typename Derived> inline Eigen::MatrixXcd SolvePoissonToRealSpace(const Eigen::MatrixBase<Derived>& chargeDensity)
		{
			return cI(SolvePoissonToReciprocalSpace(chargeDensity)).real();
		}

		//*********************************************************************************************

		// if a square matrix, I and J operators are inverse of each other
		// might be the case that it's not a square matrix, one might have only a few basis functions
		// compared with the number of sample points in the real space
		// that is not the case for the current program
		template<typename Derived> inline Eigen::MatrixXcd cI(const Eigen::MatrixBase<Derived>& in)
		{
			return FFT3D(in);
		}

		template<typename Derived> inline Eigen::MatrixXcd cIdag(const Eigen::MatrixBase<Derived>& in)
		{
			return InvFFT3D(in);
		}

		template<typename Derived> inline Eigen::MatrixXcd cJ(const Eigen::MatrixBase<Derived>& in)
		{
			const Eigen::MatrixXcd out = InvFFT3D(in);

			const double norm = 1. / realSpaceCell.Samples();

			return norm * out;
		}


		template<typename Derived> inline Eigen::MatrixXcd cJdag(const Eigen::MatrixBase<Derived>& in)
		{
			const Eigen::MatrixXcd out = FFT3D(in);
		
			const double norm = 1. / realSpaceCell.Samples();
				
			return norm * out;
		}

		template<typename Derived> inline Eigen::MatrixXcd cJdagOcJ(const Eigen::MatrixBase<Derived>& in)
		{
			return realSpaceCell.SampleVolume() * in;
		}

		//*********************************************************************************************


		// overlap operator - it is diagonal for plane waves basis
		template<typename Derived> inline Eigen::MatrixXcd O(const Eigen::MatrixBase<Derived>& in) const
		{
			return realSpaceCell.Volume() * in;
		}

		// Laplacian operator in reciprocal space
		template<typename Derived> inline Eigen::MatrixXcd L(const Eigen::MatrixBase<Derived>& in) const
		{
			return -realSpaceCell.Volume() * (reciprocalCell.LatticeVectorsSquaredMagnitude * Eigen::MatrixXcd::Ones(1, in.cols())).cwiseProduct(in);
		}

		// inverse Laplacian operator in reciprocal space
		template<typename Derived> Eigen::MatrixXcd Linv(const Eigen::MatrixBase<Derived>& in) const
		{
			Eigen::MatrixXcd out = -1. / realSpaceCell.Volume() * in.cwiseQuotient(reciprocalCell.LatticeVectorsSquaredMagnitude);

			out(0) = 0;

			return out;
		}

		template<typename Derived> inline Eigen::MatrixXcd diagouter(const Eigen::MatrixBase<Derived>& A, const Eigen::MatrixBase<Derived>& B) const
		{
			return A.cwiseProduct(B.conjugate()).rowwise().sum();			
		}

		template<typename Derived> inline Eigen::MatrixXcd Diagprod(const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& B) const
		{
			return (a * Eigen::MatrixXcd::Ones(1, B.cols())).cwiseProduct(B);
		}


		inline Eigen::MatrixXcd getn(const Eigen::MatrixXcd& Psi)
		{
			Eigen::MatrixXcd IPsi = cI(Psi.col(0));
			Eigen::MatrixXcd n = IPsi.cwiseProduct(IPsi.conjugate());

			for (unsigned int col = 1; col < Psi.cols(); ++col)
			{
				IPsi = cI(Psi.col(col));
				n += IPsi.cwiseProduct(IPsi.conjugate());
			}

			return f * n;
		}

		template<typename Derived> inline double getE(const Eigen::MatrixBase<Derived>& W)
		{
			// U is the overlap between wavefunctions - they might not be orthogonal
			const Eigen::MatrixXcd Uinv = (W.adjoint() * O(W)).inverse();
			const Eigen::MatrixXcd IW = cI(W);

			const Eigen::MatrixXcd n = f * diagouter((IW * Uinv).eval(), IW); // f * diag(density matrix) = f * diag(I * P * Idag)

			const double KineticEnergy = -0.5 * f * diagouter(L(W * Uinv), W).sum().real();
			const double PotentialEnergy = (dualV.adjoint() * n + n.adjoint() * (0.5 * cJdag(O(SolvePoissonToReciprocalSpace(n))) + cJdagOcJ(ExchCor::exc(n))))(0).real();
																							   
			return KineticEnergy + PotentialEnergy;
		}

		template<typename Derived> inline Eigen::MatrixXcd H(const Eigen::MatrixBase<Derived>& W)
		{
			// U is the overlap between wavefunctions - they might not be orthogonal
			const Eigen::MatrixXcd Uinv = (W.adjoint() * O(W)).inverse();
			const Eigen::MatrixXcd IW = cI(W);

			const Eigen::MatrixXcd n = f * diagouter((IW * Uinv).eval(), IW); // f * diag(density matrix) = f * diag(I * P * Idag)

			const Eigen::MatrixXcd Veff = dualV + cJdag(O(SolvePoissonToReciprocalSpace(n))) + cJdagOcJ(ExchCor::exc(n)) + ExchCor::excDeriv(n).cwiseProduct(cJdagOcJ(n));

			return -0.5 * L(W) + cIdag(Diagprod(Veff, IW));
		}

		template<typename Derived> inline Eigen::MatrixXcd getgrad(const Eigen::MatrixBase<Derived>& W)
		{
			const Eigen::MatrixXcd Wadj = W.adjoint();
			const Eigen::MatrixXcd OW = O(W);
			const Eigen::MatrixXcd Uinv = (Wadj * OW).inverse();
			const Eigen::MatrixXcd HW = H(W);
  
			return f * (HW - (OW * Uinv).eval() * (Wadj * HW).eval()) * Uinv;
		}

		template<typename Derived> inline Eigen::MatrixXcd orthogonalize(const Eigen::MatrixBase<Derived>& W) const
		{
			const Eigen::MatrixXcd U = W.adjoint() * O(W);

			return W * U.sqrt().inverse();
		}


		template<typename Derived> inline std::pair<Eigen::MatrixXcd, Eigen::VectorXcd> getPsi(const Eigen::MatrixBase<Derived>& W)
		{
			const Eigen::MatrixXcd Y = orthogonalize(W);
			const Eigen::MatrixXcd mu = Y.adjoint() * H(Y); // <Yi|H|Yj> matrix

			eigensolver.compute(mu);
			
			assert(eigensolver.info() == Eigen::ComputationInfo::Success);

			const Eigen::MatrixXcd Psi = Y * eigensolver.eigenvectors();

			return std::make_pair(Psi, eigensolver.eigenvalues());
		}


		// preconditioner
		template<typename Derived> inline Eigen::MatrixXcd K(const Eigen::MatrixBase<Derived>& in) const
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());

			for (unsigned int i = 0; i < in.cols(); ++i)
				out.col(i) = in.col(i).cwiseQuotient(Eigen::MatrixXcd::Ones(in.rows(), 1) + reciprocalCell.LatticeVectorsSquaredMagnitude);

			 return out;
		}


		RealSpaceCell realSpaceCell;
		ReciprocalSpaceCell reciprocalCell;

		Fourier::FFT fft;

		unsigned int f; //occupancy, default 1, for the quantum dot 2
	
	protected:

		template<typename Derived> inline Eigen::MatrixXcd FFT3D(const Eigen::MatrixBase<Derived>& in)
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());
		
			for (unsigned int i = 0; i < in.cols(); ++i)
				fft.fwd(in.col(i).data(), out.col(i).data(), realSpaceCell.GetSamples().X, realSpaceCell.GetSamples().Y, realSpaceCell.GetSamples().Z);
				
			return out;
		}


		template<typename Derived> inline Eigen::MatrixXcd InvFFT3D(const Eigen::MatrixBase<Derived>& in)
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());
		
			for (unsigned int i = 0; i < in.cols(); ++i)
				fft.inv(in.col(i).data(), out.col(i).data(), realSpaceCell.GetSamples().X, realSpaceCell.GetSamples().Y, realSpaceCell.GetSamples().Z);
				
			return out;
		}


	private:
		Eigen::MatrixXcd dualV;
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver;
	};


}