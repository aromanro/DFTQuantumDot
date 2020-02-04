#pragma once
#include "DftSolver.h"

namespace DFT {

	namespace SolutionFinders {

		namespace BaseClasses {

			template<class Solver> class SteepestDescent
			{
			public:
				SteepestDescent(Solver& theSolver) : solver(theSolver) { }

				Eigen::MatrixXcd Descend(const Eigen::MatrixXcd& W, unsigned int iter = 600, double alpha = 0.00003)
				{
					Eigen::MatrixXcd res = W;

					for (unsigned int i = 0; i < iter; ++i)
					{
						res -= alpha * solver.getgrad(res);

						//TRACE(L"E: %f, for step: %d\n", solver.getE(res), i);
					}

					/*
					CString str;
					str.Format(L"E: %f", solver.getE(res));

					AfxMessageBox(str);
					*/

					return res;
				}

			protected:
				Solver& solver;
			};


			template<class Solver, class Direction> class Minimization
			{
			public:
				Minimization(Solver& theSolver) : solver(theSolver) {}


				Eigen::MatrixXcd Descend(const Eigen::MatrixXcd& W, unsigned int iter = 600, double alphat = 0.00003)
				{
					double alpha = 0;
					double beta = 0;

					Eigen::MatrixXcd res = W;
					Eigen::MatrixXcd gradient, dir, gradTrial, gradDif, trial, oldDir, oldGrad;

					for (unsigned int i = 0; i < iter; ++i)
					{
						// save the old values for direction and gradient
						oldDir = dir;
						oldGrad = gradient;

						// get the gradient and direction for current position
						// for conjugate gradient the direction will be adjusted using the old direction with some beta depending on the method
						gradient = solver.getgrad(res);
						dir = Direction::GetDirection(solver, gradient); // Set search direction

						// the cosine of the angle between the gradient at current point and the vector for the last step
						//TRACE(L"Angle cos: %f\n", i ? Solver::Dot(gradient, oldDir) / sqrt(Solver::Dot(gradient, gradient) * Solver::Dot(oldDir, oldDir)) : 1);

						if (i)
						{
							// beta is zero for 'pure' line minimization algorithms
							// non zero for conjugate gradient methods
							beta = Direction::GetBeta(gradient, oldGrad, dir, oldDir);
							dir += beta * oldDir;
						}

						trial = res + alphat * dir; // look ahead
						gradTrial = solver.getgrad(trial); // Evaluate gradient at trial step
						gradDif = gradient - gradTrial;

						double dotdif = Solver::Dot(gradDif, dir);
						if (abs(dotdif) < 1E-20) dotdif = 1E-20 * (dotdif < 0 ? -1 : 1);

						alpha = alphat * Solver::Dot(gradient, dir) / dotdif; // Compute estimate of best step				

						res += alpha * dir;

						//TRACE(L"E: %f, for step: %d\n", solver.getE(res), i);
					}

					/*
					CString str;
					str.Format(L"E: %f", solver.getE(res));
					AfxMessageBox(str);
					*/

					return res;
				}

			protected:
				Solver& solver;
			};


		}

		namespace Helpers {

			template<class Solver> class GradientDir
			{
			public:
				static Eigen::MatrixXcd GetDirection(Solver& /*solver*/, const Eigen::MatrixXcd& gradient)
				{
					return -gradient;
				}
			};

			template<class Solver> class KDir
			{
			public:
				static Eigen::MatrixXcd GetDirection(Solver& solver, const Eigen::MatrixXcd& gradient)
				{
					return -solver.K(gradient);
				}
			};


			template<class Solver> class GradDirection : public GradientDir<Solver>
			{
			public:
				static double GetBeta(const Eigen::MatrixXcd& /*gradient*/, const Eigen::MatrixXcd& /*oldGradient*/, const Eigen::MatrixXcd& /*dir*/, const Eigen::MatrixXcd& /*oldDir*/)
				{
					return 0;
				}
			};

			template<class Solver> class KDirection : public KDir<Solver>
			{
			public:
				static double GetBeta(const Eigen::MatrixXcd& /*gradient*/, const Eigen::MatrixXcd& /*oldGradient*/, const Eigen::MatrixXcd& /*dir*/, const Eigen::MatrixXcd& /*oldDir*/)
				{
					return 0;
				}
			};


			template<class Solver> class FletcherReeves : public KDir<Solver>
			{
			public:
				static double GetBeta(const Eigen::MatrixXcd& gradient, const Eigen::MatrixXcd& oldGradient, const Eigen::MatrixXcd& dir, const Eigen::MatrixXcd& oldDir)
				{
					return Solver::Dot(gradient, dir) / Solver::Dot(oldGradient, oldDir);
				}
			};

			template<class Solver> class PolakRibiere : public KDir<Solver>
			{
			public:
				static double GetBeta(const Eigen::MatrixXcd& gradient, const Eigen::MatrixXcd& oldGradient, const Eigen::MatrixXcd& dir, const Eigen::MatrixXcd& oldDir)
				{
					const Eigen::MatrixXcd difG = gradient - oldGradient;

					return Solver::Dot(difG, dir) / Solver::Dot(oldGradient, oldDir);
				}
			};

			template<class Solver> class HestenesStiefel : public KDir<Solver>
			{
			public:
				static double GetBeta(const Eigen::MatrixXcd& gradient, const Eigen::MatrixXcd& oldGradient, const Eigen::MatrixXcd& dir, const Eigen::MatrixXcd& oldDir)
				{
					const Eigen::MatrixXcd difG = gradient - oldGradient;

					return -Solver::Dot(difG, dir) / Solver::Dot(difG, oldDir);
				}
			};

		}




		template<class Solver> class LineMinimization : public BaseClasses::Minimization<Solver, Helpers::GradDirection<Solver>>
		{
		public:
			LineMinimization(Solver& solver) : BaseClasses::Minimization<Solver, Helpers::GradDirection<Solver>>(solver) {}
		};

		template<class Solver> class PreconditionedLineMinimization : public BaseClasses::Minimization<Solver, Helpers::KDirection<Solver>>
		{
		public:
			PreconditionedLineMinimization(Solver& solver) : BaseClasses::Minimization<Solver, Helpers::KDirection<Solver>>(solver) {}
		};


		template<class Solver> class FletcherReevesConjugateGradient : public BaseClasses::Minimization<Solver, Helpers::FletcherReeves<Solver>>
		{
		public:
			FletcherReevesConjugateGradient(Solver& solver) : BaseClasses::Minimization<Solver, Helpers::FletcherReeves<Solver>>(solver) {}
		};


		template<class Solver> class PolakRibiereConjugateGradient : public BaseClasses::Minimization<Solver, Helpers::PolakRibiere<Solver>>
		{
		public:
			PolakRibiereConjugateGradient(Solver& solver) : BaseClasses::Minimization<Solver, Helpers::PolakRibiere<Solver>>(solver) {}
		};


		template<class Solver> class HestenesStiefelConjugateGradient : public BaseClasses::Minimization<Solver, Helpers::HestenesStiefel<Solver>>
		{
		public:
			HestenesStiefelConjugateGradient(Solver& solver) : BaseClasses::Minimization<Solver, Helpers::HestenesStiefel<Solver>>(solver) {}
		};

	}

}

