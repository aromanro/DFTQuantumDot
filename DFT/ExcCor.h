#pragma once

#include "ExcCorBase.h"

#include <vector>

namespace DFT {

	// see https://aip.scitation.org/doi/10.1063/1.4958669
	class ChachiyoExchCorParam
	{
	public:
		static constexpr double b = 20.4562557;
		static constexpr double b1 = 27.4203609;
	};

	// see https://aip.scitation.org/doi/10.1063/1.4964758
	class ChachiyoExchCorImprovedParam
	{
	public:
		static constexpr double b = 21.7392245;
		static constexpr double b1 = 28.3559732;
	};

	template<class Params> class ChachiyoExchCor : public ExcCorBase
	{
	protected:
		static constexpr double a = (M_LN2 - 1.) / (2. * M_PI * M_PI);
		static constexpr double b = Params::b;

		static constexpr double a1 = (M_LN2 - 1.) / (4. * M_PI * M_PI);
		static constexpr double b1 = Params::b1;

		static constexpr double fourM_PI = 4. * M_PI;
		static constexpr double threeDivM_PI = 3. / M_PI;

	public:
		static Eigen::MatrixXcd Vexc(const Eigen::MatrixXcd& n)
		{
			static const double	X1 = pow(3. / (2. * M_PI), 2. / 3.);  // Exchange energy coefficient
			assert(n.cols() == 1);

			Eigen::MatrixXcd res(n.rows(), 1);

			for (int i = 0; i < sz; ++i)
			{
				const double ro = n(i, 0).real();
				if (ro < 1E-18)
				{
					res(i, 0) = 0;
					continue;
				}

				const double rs = pow(3. / (fourM_PI * ro), 1. / 3.);

				// exchange
				res(i, 0) = -X1 / rs;

				// correlation
				const double bprs = b / rs;
				const double bprs2 = bprs / rs;

				res(i, 0) += a * log(1. + bprs + bprs / rs)
					- a / (1. + bprs + bprs2) * (bprs + 2. * bprs2) * rs / 3.;
			}

			return res;
		}


		static Eigen::MatrixXcd excDeriv(const Eigen::MatrixXcd& n)
		{
			static const double X1 = 0.25 * pow(3. / (2. * M_PI), 2. / 3.);  // Exchange energy coefficient
			assert(n.cols() == 1);

			Eigen::MatrixXcd res(n.rows(), 1);

			for (int i = 0; i < sz; ++i)
			{
				const double ro = n(i, 0).real();
				if (ro < 1E-18)
				{
					res(i, 0) = 0;
					continue;
				}

				const double rs = pow(3. / (fourM_PI * ro), 1. / 3.);

				// exchange
				res(i, 0) = X1 / rs;

				// correlation
				const double bprs = b / rs;
				const double bprs2 = bprs / rs;

				res(i, 0) += a / (1. + bprs + bprs2) * (bprs + 2. * bprs2) * rs / 3.;
			}

			return res;
		}
	};

}
