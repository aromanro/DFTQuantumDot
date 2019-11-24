#pragma once


#include "DftSolverBase.h"

namespace DFT {

	// 'curiously recurring template pattern' - see SolvePoissonToRealSpace, getgrad and getPsi implementations in base class
	template<class ExchCor> class DftSolver : public DftSolverBase<ExchCor, DftSolver<ExchCor>>
	{
	public:
		using _Mybase = DftSolverBase<ExchCor, DftSolver<ExchCor>>;

		DftSolver(double dim1, double dim2, double dim3, unsigned int samples1, unsigned int samples2, unsigned int samples3, unsigned int Ns)
			: _Mybase(dim1, dim2, dim3, samples1, samples2, samples3, Ns)
		{
		}


		inline Eigen::MatrixXcd GetRandomState()
		{
			Eigen::MatrixXcd W = Eigen::MatrixXd::Random(_Mybase::realSpaceCell.Samples(), _Mybase::m_Ns);
			return _Mybase::orthogonalize(W);
		}

		inline Eigen::MatrixXcd GetInitialState() const
		{
			return _Mybase::orthogonalize(Eigen::MatrixXd::Random(_Mybase::realSpaceCell.Samples(), _Mybase::m_Ns));
		}

		
		inline Eigen::MatrixXcd GetInitialState(double molSize)
		{
			Eigen::MatrixXcd W = Eigen::MatrixXd::Random(_Mybase::realSpaceCell.Samples(), _Mybase::m_Ns);

			const unsigned int size = _Mybase::realSpaceCell.Samples();
			const Vector3D<double> center = _Mybase::realSpaceCell.GetSize() / 2.;

			for (unsigned int i = 0; i < size; ++i)
			{
				const Vector3D<double> distanceVector(_Mybase::realSpaceCell.SamplePoints(i) - center);
				const double len = distanceVector.Length();

				if (len < molSize / 2)
				{
					for (unsigned int state = 0; state < _Mybase::m_Ns; ++state)
						W(i, state) = 0;
				}
			}


			for (unsigned int i = 0; i < _Mybase::m_Ns; ++i)
				W.col(i).stableNormalize();

			W = _Mybase::orthogonalize(_Mybase::cJdag(W));

			return W;
		}
		

		inline Eigen::MatrixXcd GetInitialState(const Eigen::MatrixXcd& Wapprox, unsigned int res)
		{
			Eigen::MatrixXcd W(_Mybase::realSpaceCell.Samples(), _Mybase::m_Ns);

			const Vector3D<unsigned int>& samples = _Mybase::realSpaceCell.GetSamples();

			for (unsigned int i = 0; i < _Mybase::m_Ns; ++i)
			{
				unsigned int index = 0;
				for (unsigned int val1 = 0; val1 < samples.X; ++val1)
					for (unsigned int val2 = 0; val2 < samples.Y; ++val2)
						for (unsigned int val3 = 0; val3 < samples.Z; ++val3)
						{
							const unsigned int p1 = (unsigned int)floor(double(val1) / samples.X * res);
							const unsigned int p2 = (unsigned int)floor(double(val2) / samples.Y * res);
							const unsigned int p3 = (unsigned int)floor(double(val3) / samples.Z * res);

							const unsigned int approxindex = (p1 * res + p2) * res + p3;

							W(index, i) = Wapprox(approxindex, i);
							++index;
						}

				W.col(i).stableNormalize();
			}

			W = _Mybase::orthogonalize(_Mybase::cJdag(W));

			return W;
		}
		


		//*********************************************************************************************

		// if a square matrix, I and J operators are inverse of each other
		// might be the case that it's not a square matrix, one might have only a few basis functions
		// compared with the number of sample points in the real space
		// that is not the case for the current class
		template<typename Derived> inline Eigen::MatrixXcd cI(const Eigen::MatrixBase<Derived>& in)
		{
			return _Mybase::FFT3D(in);
		}

		template<typename Derived> inline Eigen::MatrixXcd cIdag(const Eigen::MatrixBase<Derived>& in)
		{
			return _Mybase::InvFFT3D(in);
		}


		//*********************************************************************************************


		// Laplacian operator in reciprocal space
		template<typename Derived> inline Eigen::MatrixXcd L(const Eigen::MatrixBase<Derived>& in) const
		{
			return -_Mybase::realSpaceCell.Volume() * (_Mybase::reciprocalCell.LatticeVectorsSquaredMagnitude * Eigen::MatrixXcd::Ones(1, in.cols())).cwiseProduct(in);
		}




		template<typename Derived> inline double getE(const Eigen::MatrixBase<Derived>& W)
		{
			// U is the overlap between wavefunctions - they might not be orthogonal
			const Eigen::MatrixXcd Uinv = (W.adjoint() * _Mybase::O(W)).inverse();
			const Eigen::MatrixXcd IW = cI(W);

			const Eigen::MatrixXcd n = _Mybase::f * _Mybase::diagouter((IW * Uinv).eval(), IW); // f * diag(density matrix) = f * diag(I * P * Idag)

			const double KineticEnergy = -0.5 * _Mybase::f * _Mybase::diagouter(L(W * Uinv), W).sum().real();
			const double PotentialEnergy = (_Mybase::dualV.adjoint() * n + n.adjoint() * (0.5 * _Mybase::cJdag(_Mybase::O(_Mybase::SolvePoissonToReciprocalSpace(n))) + _Mybase::cJdagOcJ(_Mybase::excCor.exc(n))))(0).real();

			return KineticEnergy + PotentialEnergy;
		}

		template<typename Derived> inline Eigen::MatrixXcd H(const Eigen::MatrixBase<Derived>& W)
		{
			// U is the overlap between wavefunctions - they might not be orthogonal
			const Eigen::MatrixXcd Uinv = (W.adjoint() * _Mybase::O(W)).inverse();
			const Eigen::MatrixXcd IW = cI(W);

			const Eigen::MatrixXcd n = _Mybase::f * _Mybase::diagouter((IW * Uinv).eval(), IW); // f * diag(density matrix) = f * diag(I * P * Idag)

			const Eigen::MatrixXcd Veff = _Mybase::dualV + _Mybase::cJdag(_Mybase::O(_Mybase::SolvePoissonToReciprocalSpace(n))) + _Mybase::cJdagOcJ(_Mybase::excCor.exc(n)) + _Mybase::excCor.excDeriv(n).cwiseProduct(_Mybase::cJdagOcJ(n));

			return -0.5 * L(W) + cIdag(_Mybase::Diagprod(Veff, IW));
		}

		// preconditioner
		template<typename Derived> inline Eigen::MatrixXcd K(const Eigen::MatrixBase<Derived>& in) const
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());

			for (unsigned int i = 0; i < in.cols(); ++i)
				out.col(i) = in.col(i).cwiseQuotient(Eigen::MatrixXcd::Ones(in.rows(), 1) + _Mybase::reciprocalCell.LatticeVectorsSquaredMagnitude);

			return out;
		}
	};

}