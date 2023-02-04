#pragma once
#include "SimpleDescentBase.h"

namespace DFT {

	namespace SolutionFinders {

		namespace Helpers {

			template<class Solver> class KDir
			{
			public:
				static Eigen::MatrixXcd GetDirection(Solver& solver, const Eigen::MatrixXcd& gradient)
				{
					return -solver.K(gradient);
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

