#pragma once

#include "DftSolverBase.h"


namespace DFT {

	// 'curiously recurring template pattern' - see SolvePoissonToRealSpace, getgrad and getPsi implementations in base class
	template<class ExchCor> class DftSolverCompressed : public DftSolverBase<ExchCor, DftSolverCompressed<ExchCor>>
	{
	public:
		using _Mybase = DftSolverBase<ExchCor, DftSolverCompressed<ExchCor>>;

		DftSolverCompressed(double dim1, double dim2, double dim3, unsigned int samples1, unsigned int samples2, unsigned int samples3, unsigned int Ns)
			: _Mybase(dim1, dim2, dim3, samples1, samples2, samples3, Ns)
		{
			fullcIfull = Eigen::MatrixXcd::Zero(_Mybase::realSpaceCell.Samples(), Ns);
			fullcIcol = Eigen::MatrixXcd::Zero(_Mybase::realSpaceCell.Samples(), 1);
		}

		inline Eigen::MatrixXcd GetRandomState()
		{
			Eigen::MatrixXcd W = Eigen::MatrixXd::Random(_Mybase::reciprocalCell.active.size(), _Mybase::m_Ns);
			return _Mybase::orthogonalize(W);
		}

		inline Eigen::MatrixXcd GetInitialState() const
		{
			return _Mybase::orthogonalize(Eigen::MatrixXd::Random(_Mybase::reciprocalCell.active.size(), _Mybase::m_Ns));
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
				
				if (len > molSize/2)
				{					
					for (unsigned int state = 0; state < _Mybase::m_Ns; ++state)
						W(i, state) = 0;
				}				
			}

			
			for (unsigned int i = 0; i < _Mybase::m_Ns; ++i)
				W.col(i).stableNormalize();

			const Eigen::MatrixXcd full = _Mybase::FFT3D(W);

			Eigen::MatrixXcd outcIdag(_Mybase::reciprocalCell.active.size(), _Mybase::m_Ns);

			for (unsigned int col = 0; col < _Mybase::m_Ns; ++col)
				for (unsigned int i = 0; i < _Mybase::reciprocalCell.active.size(); ++i)
					outcIdag(i, col) = full(_Mybase::reciprocalCell.active[i], col);
			

			for (unsigned int i = 0; i < _Mybase::m_Ns; ++i)
				outcIdag.col(i).stableNormalize();

			return _Mybase::orthogonalize(outcIdag);
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
			
			const Eigen::MatrixXcd full = _Mybase::FFT3D(W);

			Eigen::MatrixXcd outcIdag(_Mybase::reciprocalCell.active.size(), _Mybase::m_Ns);

			for (unsigned int col = 0; col < _Mybase::m_Ns; ++col)
			{
				for (unsigned int i = 0; i < _Mybase::reciprocalCell.active.size(); ++i)
					outcIdag(i, col) = full(_Mybase::reciprocalCell.active[i], col);
				outcIdag.col(col).stableNormalize();
			}

			return _Mybase::orthogonalize(outcIdag);
		}



		//*********************************************************************************************

		// if a square matrix, I and J operators are inverse of each other
		// as in cJ = cI^-1 and cJdag = cIdag^-1
		// preserving the normalization (if needed) is done in cJ and cJdag (see their implementation in DftSolverBase for the details)
		// as FFTW doesn't do it
		// here we have the case that it's not a square matrix, we have fewer basis functions
		// compared with the number of sample points in the real space

		inline Eigen::MatrixXcd cI(const Eigen::MatrixXcd& in)
		{
			if (in.rows() == _Mybase::realSpaceCell.Samples()) return _Mybase::FFT3D(in);

			//Eigen::MatrixXcd fullcI = Eigen::MatrixXcd::Zero(realSpaceCell.Samples(), in.cols());
			Eigen::MatrixXcd& fullcI = (in.cols() > 1 ? fullcIfull : fullcIcol);

			for (unsigned int col = 0; col < in.cols(); ++col)
				for (unsigned int i = 0; i < _Mybase::reciprocalCell.active.size(); ++i)
					fullcI(_Mybase::reciprocalCell.active[i], col) = in(i, col);
			
			return _Mybase::FFT3D(fullcI);
		}

		inline Eigen::MatrixXcd cIdag(const Eigen::MatrixXcd& in)
		{
			const Eigen::MatrixXcd full = _Mybase::InvFFT3D(in);

			Eigen::MatrixXcd outcIdag(_Mybase::reciprocalCell.active.size(), in.cols());

			for (unsigned int col = 0; col < in.cols(); ++col)
				for (unsigned int i = 0; i < _Mybase::reciprocalCell.active.size(); ++i)
					outcIdag(i, col) = full(_Mybase::reciprocalCell.active[i], col);

			return outcIdag;
		}



		//*********************************************************************************************




		// Laplacian operator in reciprocal space
		inline Eigen::MatrixXcd L(const Eigen::MatrixXcd& in) const
		{
			if (static_cast<size_t>(in.rows()) == _Mybase::reciprocalCell.active.size())
				return -_Mybase::realSpaceCell.Volume() * (_Mybase::reciprocalCell.LatticeVectorsSquaredMagnitudeCompressed * Eigen::MatrixXcd::Ones(1, in.cols())).cwiseProduct(in);

			return -_Mybase::realSpaceCell.Volume() * (_Mybase::reciprocalCell.LatticeVectorsSquaredMagnitude * Eigen::MatrixXcd::Ones(1, in.cols())).cwiseProduct(in);
		}

		inline double getE(const Eigen::MatrixXcd& W)
		{
			const Eigen::MatrixXcd Y = _Mybase::orthogonalize(W);		
			const Eigen::MatrixXcd n = _Mybase::getn(Y);

			const double KineticEnergy = -0.5 * _Mybase::f * _Mybase::diagouter(L(Y), Y).sum().real();
			const double PotentialEnergy = (_Mybase::dualV.adjoint() * n + n.adjoint() * (0.5 * _Mybase::cJdag(_Mybase::O(_Mybase::SolvePoissonToReciprocalSpace(n))) + _Mybase::cJdagOcJ(_Mybase::excCor.Vexc(n))))(0).real();
																							   
			return KineticEnergy + PotentialEnergy;
		}

		inline Eigen::MatrixXcd H(const Eigen::MatrixXcd& W)
		{
			const Eigen::MatrixXcd Y = _Mybase::orthogonalize(W);
			const Eigen::MatrixXcd n = _Mybase::getn(Y);

			const Eigen::MatrixXcd Veff = _Mybase::dualV + _Mybase::cJdag(_Mybase::O(_Mybase::SolvePoissonToReciprocalSpace(n))) + _Mybase::cJdagOcJ(_Mybase::excCor.Vexc(n)) + _Mybase::excCor.excDeriv(n).cwiseProduct(_Mybase::cJdagOcJ(n));

			Eigen::MatrixXcd H = -0.5 * L(Y);

			for (unsigned int col = 0; col < W.cols(); ++col)
				H.col(col) += cIdag(Veff.cwiseProduct(cI(Y.col(col))));

			return H;
		}



		// preconditioner
		inline Eigen::MatrixXcd K(const Eigen::MatrixXcd& in) const
		{
			Eigen::MatrixXcd out(in.rows(), in.cols());

			if (static_cast<size_t>(in.rows()) == _Mybase::reciprocalCell.active.size())
				for (unsigned int i = 0; i < in.cols(); ++i)
					out.col(i) = in.col(i).cwiseQuotient(Eigen::MatrixXcd::Ones(in.rows(), 1) + _Mybase::reciprocalCell.LatticeVectorsSquaredMagnitudeCompressed);
			else
				for (unsigned int i = 0; i < in.cols(); ++i)
					out.col(i) = in.col(i).cwiseQuotient(Eigen::MatrixXcd::Ones(in.rows(), 1) + _Mybase::reciprocalCell.LatticeVectorsSquaredMagnitude);

			 return out;
		}

	private:
		Eigen::MatrixXcd fullcIfull;
		Eigen::MatrixXcd fullcIcol;
	};


}