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


		void SetPotential(const Eigen::MatrixXcd& V)
		{
			dualV = cJdag(O(cJ(V)));
		}

		void SetReciprocalPotential(const Eigen::MatrixXcd& V)
		{
			dualV = cJdag(V);
		}

		template<typename Derived> static double Dot(const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b)
		{
			return (a.adjoint() * b).trace().real();
		}

		inline Eigen::MatrixXcd SolvePoissonToReciprocalSpace(const Eigen::MatrixXcd& chargeDensity)
		{
			return -4. * M_PI * Linv(O(cJ(chargeDensity)));
		}

		inline Eigen::MatrixXcd SolvePoissonToRealSpace(const Eigen::MatrixXcd& chargeDensity)
		{
			return cI(SolvePoissonToReciprocalSpace(chargeDensity));
		}

		//*********************************************************************************************

		// if a square matrix, I and J operators are inverse of each other
		// might be the case that it's not a square matrix, one might have only a few basis functions
		// compared with the number of sample points in the real space
		// that is not the case for the current program
		template<typename Derived> Eigen::MatrixXcd cI(const Eigen::MatrixBase<Derived>& in)
		{
			return FFT3D(in);
		}

		template<typename Derived> Eigen::MatrixXcd cIdag(const Eigen::MatrixBase<Derived>& in)
		{
			return InvFFT3D(in);
		}

		template<typename Derived> Eigen::MatrixXcd cJ(const Eigen::MatrixBase<Derived>& in)
		{
			Eigen::MatrixXcd out = InvFFT3D(in);

			const double norm = 1. / realSpaceCell.Samples();

			return norm * out;
		}


		template<typename Derived> Eigen::MatrixXcd cJdag(const Eigen::MatrixBase<Derived>& in)
		{
			Eigen::MatrixXcd out = FFT3D(in);
		
			const double norm = 1. / realSpaceCell.Samples();
				
			return norm * out;
		}

		//*********************************************************************************************


		// overlap operator - it is diagonal for plane waves basis
		template<typename Derived> Eigen::MatrixXcd O(const Eigen::MatrixBase<Derived>& in) const
		{
			return realSpaceCell.Volume() * in;
		}

		// Laplacian operator in reciprocal space
		template<typename Derived> Eigen::MatrixXcd L(const Eigen::MatrixBase<Derived>& in) const
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

		template<typename Derived> Eigen::MatrixXcd diagouter(const Eigen::MatrixBase<Derived>& A, const Eigen::MatrixBase<Derived>& B) const
		{
			return  A.cwiseProduct(B.conjugate()).rowwise().sum();			
		}

		template<typename Derived> Eigen::MatrixXcd Diagprod(const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& B) const
		{
			return (a * Eigen::MatrixXcd::Ones(1, B.cols())).cwiseProduct(B);
		}


		template<typename Derived> double getE(const Eigen::MatrixBase<Derived>& W)
		{
			// U is the overlap between wavefunctions - they might not be orthogonal
			const Eigen::MatrixXcd Uinv = (W.adjoint() * O(W)).inverse();
			const Eigen::MatrixXcd IW = cI(W);

			const Eigen::MatrixXcd n = f * diagouter((IW * Uinv).eval(), IW); // f * diag(density matrix) = f * diag(I * P * Idag)
			const Eigen::MatrixXcd ndag = n.adjoint();

			const Eigen::VectorXcd Phi = SolvePoissonToReciprocalSpace(n);

			const Eigen::MatrixXcd exc = ExchCor::exc(n);
			
			const Eigen::MatrixXcd EKn = -0.5 * f * diagouter(L(W * Uinv), W);

			const double KineticEnergy = EKn.sum().real();

			const Eigen::VectorXcd PhiExc = 0.5 * Phi + cJ(exc);
			const double PotentialEnergy = (dualV.adjoint() * n + ndag * cJdag(O(PhiExc)))(0).real();
																							   
			return KineticEnergy + PotentialEnergy;
		}

		template<typename Derived> Eigen::MatrixXcd H(const Eigen::MatrixBase<Derived>& W)
		{
			// U is the overlap between wavefunctions - they might not be orthogonal
			const Eigen::MatrixXcd Uinv = (W.adjoint() * O(W)).inverse();
			const Eigen::MatrixXcd IW = cI(W);

			const Eigen::MatrixXcd n = f * diagouter((IW * Uinv).eval(), IW); // f * diag(density matrix) = f * diag(I * P * Idag)

			const Eigen::MatrixXcd Phi = SolvePoissonToReciprocalSpace(n);

			const Eigen::MatrixXcd exc = ExchCor::exc(n);
			const Eigen::MatrixXcd excDeriv = ExchCor::excDeriv(n);

			const Eigen::VectorXcd PhiExc = Phi + cJ(exc);
			const Eigen::MatrixXcd Veff = dualV + cJdag(O(PhiExc)) + excDeriv.cwiseProduct(cJdag(O(cJ(n))));

			return -0.5 * L(W) + cIdag(Diagprod(Veff, cI(W)));
		}

		template<typename Derived> Eigen::MatrixXcd getgrad(const Eigen::MatrixBase<Derived>& W)
		{
			const Eigen::MatrixXcd Uinv = (W.adjoint() * O(W)).inverse();
			const Eigen::MatrixXcd HW = H(W);
  
			return f * (HW - (O(W) * Uinv).eval() * (W.adjoint() * HW).eval()) * Uinv;
		}

		template<typename Derived> Eigen::MatrixXcd orthogonalize(const Eigen::MatrixBase<Derived>& W) const
		{
			const Eigen::MatrixXcd U = W.adjoint() * O(W);

			return W * U.sqrt().inverse();
		}


		template<typename Derived> std::pair<Eigen::MatrixXcd, Eigen::VectorXcd> getPsi(const Eigen::MatrixBase<Derived>& W)
		{
			const Eigen::MatrixXcd Y = orthogonalize(W);
			const Eigen::MatrixXcd mu = Y.adjoint() * H(Y); // <Yi|H|Yj> matrix

			eigensolver.compute(mu);
			
			assert(eigensolver.info() == Eigen::ComputationInfo::Success);

			const Eigen::MatrixXcd Psi = Y * eigensolver.eigenvectors();

			return std::make_pair(Psi, eigensolver.eigenvalues());
		}


		// preconditioner
		template<typename Derived> Eigen::MatrixXcd K(const Eigen::MatrixBase<Derived>& in) const
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());

			for (int i = 0; i < in.cols(); ++i)
				out.col(i) = in.col(i).cwiseQuotient(Eigen::MatrixXcd::Ones(in.rows(), 1) + reciprocalCell.LatticeVectorsSquaredMagnitude);

			 return out;
		}


		RealSpaceCell realSpaceCell;
		ReciprocalSpaceCell reciprocalCell;

		Fourier::FFT fft;

		unsigned int f; //occupancy, default 1, for the quantum dot 2
	
	protected:

		template<typename Derived> Eigen::MatrixXcd FFT3D(const Eigen::MatrixBase<Derived>& in)
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());
		
			Eigen::VectorXcd col(in.rows());

			for (int i = 0; i < in.cols(); ++i)
			{
				fft.fwd(in.col(i).data(), col.data(), realSpaceCell.GetSamples().X, realSpaceCell.GetSamples().Y, realSpaceCell.GetSamples().Z);
				out.col(i) = col;
			}
				
			return out;
		}


		template<typename Derived> Eigen::MatrixXcd InvFFT3D(const Eigen::MatrixBase<Derived>& in)
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());
		
			Eigen::VectorXcd col(in.rows());

			for (int i = 0; i < in.cols(); ++i)
			{
				fft.inv(in.col(i).data(), col.data(), realSpaceCell.GetSamples().X, realSpaceCell.GetSamples().Y, realSpaceCell.GetSamples().Z);
				out.col(i) = col;
			}
				
			return out;
		}


	private:
		Eigen::MatrixXcd dualV;
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver;
	};


}