#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

namespace DFT {


	class  ChachiyoExchCor
	{
	public:
		static Eigen::MatrixXcd exc(const Eigen::MatrixXcd& n)
		{
			assert(n.cols() == 1);

			static const double a = (log(2.) - 1.) / (2. * M_PI * M_PI);
			static const double b = 20.4562557;

			Eigen::MatrixXcd res(n.rows(), 1);


			for (int i = 0; i < n.rows(); ++i)
			{
				const double ro = n(i, 0).real();

				// exchange
				res(i, 0) = -3. / 4. * pow(3. / M_PI * ro, 1. / 3.); // Dirac exchange

				// correlation
				const double rs = pow(3. / (4. * M_PI * ro), 1. / 3.);
				const double bprs = b / rs;

				res(i, 0) += a * log(1. + bprs + bprs / rs);
			}


			return res;
		}


		static Eigen::MatrixXcd excDeriv(const Eigen::MatrixXcd& n)
		{
			assert(n.cols() == 1);

			const double a = (log(2.) - 1.) / (2. * M_PI * M_PI);
			const double b = 20.4562557;

			Eigen::MatrixXcd res(n.rows(), 1);

			for (int i = 0; i < n.rows(); ++i)
			{
				const double ro = n(i, 0).real();

				// exchange
				res(i, 0) = -1. / 4. * pow(3. / M_PI, 1. / 3.) * pow(ro, -2. / 3.);

				// correlation
				const double rs = pow(3. / (4. * M_PI * ro), 1. / 3.);
				const double bprs = b / rs;
				const double bprs2 = bprs / rs;
				const double bprs3 = bprs2 / rs;

				res(i, 0) += a * (bprs2 + 2. * bprs3) / (1. + bprs + bprs2) * rs / (3. * ro);
			}

			return res;
		}

	};

}
