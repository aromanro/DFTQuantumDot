#pragma once

#include <cassert>
#include <vector>

#include "ExcCorBase.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace DFT {

	// Vosko-Wilk-Nusair 
	// see Richard M. Martin, Electronic Structure, Basic Theory and Practical Methods
	// also here: https://www.nist.gov/pml/atomic-reference-data-electronic-structure-calculations-exchange-term
	// and here: https://www.nist.gov/pml/atomic-reference-data-electronic-structure-calculations/atomic-reference-data-electronic-6-3

	class VWNExchCor : public ExcCorBase
	{
	protected:
		static constexpr double fourM_PI = 4. * M_PI;

		// values for 'paramagnetic' variant (used for LDA and LSDA)
		static constexpr double AP = 0.0310907; // actually 0.5 * A, because they are originally given in Rydbergs
		static constexpr double y0P = -0.10498;
		static constexpr double bP = 3.72744;
		static constexpr double cP = 12.93532;
		static constexpr double Y0P = y0P * y0P + bP * y0P + cP;

		// values for 'feromagnetic' variant (useful for LSDA)
		static constexpr double AF = 0.01554535; // actually 0.5 * A
		static constexpr double y0F = -0.325;
		static constexpr double bF = 7.06042;
		static constexpr double cF = 18.0578;
		static constexpr double Y0F = y0F * y0F + bF * y0F + cF;

		// values for 'spin stiffness' (useful for LSDA)
		static constexpr double Aalpha = -1. / (6. * M_PI * M_PI); // actually 0.5 * A
		static constexpr double y0alpha = -0.0047584;
		static constexpr double balpha = 1.13107;
		static constexpr double calpha = 13.0045;
		static constexpr double Y0alpha = y0alpha * y0alpha + balpha * y0alpha + calpha;

		inline static double F(double y /*sqrt(rs)*/, double dify /*y - y0*/, double A, double y0, double b, double c, double Y0, double Y /* y^2 + by + c */)
		{
			const double Q = sqrt(4 * c - b * b);
			const double twoyb = 2. * y + b;
			const double atanQ = atan(Q / twoyb);

			return A * (log(y * y / Y) + 2. * b / Q * atanQ - b * y0 / Y0 * (log(dify * dify / Y) + 2. * (b + 2. * y0) / Q * atanQ)); // B.5
		}

		inline static double ecDif(double y, double dify, double A, double y0, double b, double c, double Y0, double Y)
		{
			return A * (c * dify - b * y0 * y) / (dify * Y); // B.6

			// alternate formula to check if getting better results (not really):
			/*
			const double d = c * y0;
			const double b1 = (b * y0 - c) / d;
			const double b2 = (y0 - b) / d;
			const double b3 = -1. / d;
			const double y2 = y * y;
			const double y3 = y2 * y;

			return A * (1. + b1 * y) / (1. + b1 * y + b2 * y2 + b3 * y3);
			*/
		}

	public:
		//**********************************************************************************************************************************
		// LSD code
		//**********************************************************************************************************************************
		static Eigen::MatrixXcd Vexc(const Eigen::MatrixXcd& n)
		{
			static const double	X1 = pow(3. / (2. * M_PI), 2. * aThird);  // Exchange energy coefficient
			assert(n.cols() == 1);

			Eigen::MatrixXcd res(n.rows(), 1);

			for (int i = 0; i < n.size(); ++i)
			{
				const double ro = n(i, 0).real();
				if (ro < 1E-18)
				{
					res(i, 0) = 0;
					continue;
				}

				const double rs = pow(3. / (fourM_PI*ro), aThird);
				const double y = sqrt(rs);
				const double Y = y * y + bP * y + cP;
				const double dify = y - y0P;

				res(i, 0) = -X1 / rs // exchange term
					//the following make the Vc as in B.1
					+ F(y, dify, AP, y0P, bP, cP, Y0P, Y) // B.5
					- aThird * ecDif(y, dify, AP, y0P, bP, cP, Y0P, Y); // B.6
			}

			return res;
		}



		static Eigen::MatrixXcd excDeriv(const Eigen::MatrixXcd& n)
		{
			static const double X1 = 0.25 * pow(3. / (2.*M_PI), 2. * aThird);  // Exchange energy coefficient
			assert(n.cols() == 1);

			Eigen::MatrixXcd res(n.rows(), 1);

			for (int i = 0; i < n.size(); ++i)
			{
				const double ro = n(i, 0).real();
				if (ro < 1E-18)
				{
					res(i, 0) = 0;
					continue;
				}

				const double rs = pow(3. / (fourM_PI * ro), aThird);

				const double y = sqrt(rs);
				const double Y = y * y + bP * y + cP;
				const double dify = y - y0P;

				res(i, 0) = X1 / rs // exchange term
					+ aThird * ecDif(y, dify, AP, y0P, bP, cP, Y0P, Y); // B.6
			}

			return res;
		}

		//**********************************************************************************************************************************
		// LSDA code
		//**********************************************************************************************************************************

		static Eigen::MatrixXcd Vexc(const Eigen::MatrixXcd& na, const Eigen::MatrixXcd& nb, Eigen::MatrixXcd& va, Eigen::MatrixXcd& vb)
		{
			assert(na.cols() == 1 && nb.cols() == 1 && va.cols() == 1 && vb.cols() == 1);

			int sz = static_cast<int>(na.rows());
			if (sz != nb.rows()) return {};

			static const double	X1 = pow(3. / (2. * M_PI), 2. * aThird);  // Exchange energy coefficient - see eq 4 NIST, but it's arranged
			static const double X2 = pow(2., aThird);
			static const double X12 = X1 * X2;
			static const double fdd = 4. / (9. * (pow(2., aThird) - 1.));

			Eigen::MatrixXcd res(sz, 1);

			va.resize(sz, 1);
			vb.resize(sz, 1);

			for (int i = 0; i < sz; ++i)
			{
				const double roa = na(i, 0).real();
				const double rob = nb(i, 0).real();
				const double n = roa + rob;
				if (n < 1E-18)
				{
					res(i, 0) = va(i, 0) = vb(i, 0) = 0;
					continue;
				}

				const double rs = pow(3. / (fourM_PI * n), aThird); // eq 2 NIST
				const double rsa = pow(3. / (fourM_PI * roa), aThird); // eq 2 NIST
				const double rsb = pow(3. / (fourM_PI * rob), aThird); // eq 2 NIST

				const double exp = -X1 / rs; // see eq 4 NIST
				const double exf = X2 * exp; // see eq 4 NIST but ef = 2^1/3 * ep
				const double exdif = exf - exp;

				const double exfa = -X12 / rsa;
				const double exfb = -X12 / rsb;

				const double zeta = (roa - rob) / n; // eq 3 NIST				
				const double zeta3 = zeta * zeta * zeta;
				const double zeta4 = zeta3 * zeta;

				const double fval = f(zeta);
				const double dfval = df(zeta);

				const double y = sqrt(rs);

				// paramagnetic values
				const double YP = y * (y + bP) + cP;
				const double difyP = y - y0P;
				const double ecp = F(y, difyP, AP, y0P, bP, cP, Y0P, YP); // B.5

				// ferromagnetic values
				const double YF = y * (y + bF) + cF;
				const double difyF = y - y0F;
				const double ecf = F(y, difyF, AF, y0F, bF, cF, Y0F, YF); // B.5

				// spin stiffness values
				const double YA = y * (y + balpha) + calpha;
				const double difyA = y - y0alpha;
				const double eca = F(y, difyA, Aalpha, y0alpha, balpha, calpha, Y0alpha, YA); // B.5

				// derivatives for the above
				const double ecpd = ecDif(y, difyP, AP, y0P, bP, cP, Y0P, YP);
				const double ecfd = ecDif(y, difyF, AF, y0F, bF, cF, Y0F, YF);
				const double ecad = ecDif(y, difyA, Aalpha, y0alpha, balpha, calpha, Y0alpha, YA);

				const double deltaecfp = ecf - ecp; // eq 10 NIST
				const double beta = fdd * deltaecfp / eca - 1.; // eq. 9 NIST
				const double opbz4 = 1. + beta * zeta4;
				const double interp = fval / fdd * opbz4; // eq 8 NIST without alphac
				const double deltaec = eca * interp; // eq. 8 NIST

				const double betad = fdd / eca * (ecfd - ecpd - ecad * deltaecfp / eca);
				const double interpd = fval / fdd * zeta4 * betad;

				// derivative with respect to rs
				const double deriv = aThird * (ecpd // paramagnetic part
					+ ecad * interp + eca * interpd);

				res(i, 0) =
					// the following two terms sum make the eq 7 from NIST
					// paramagnetic part:
					ecp
					// the polarization part
					+ deltaec
					// this is the same as above, actually, just checking to see if there is a mistake:
					//+ eca * fval / fdd * (1. - zeta4)
					//+ deltaecfp * fval * zeta4		

					// derivative
					-deriv;

				// derivative with respect to zeta
				const double dterm = eca / fdd * (4. * beta * zeta3 * fval + opbz4 * dfval);

				va(i, 0) = exfa + res(i, 0) + (1. - zeta) * dterm;
				vb(i, 0) = exfb + res(i, 0) - (1. + zeta) * dterm;

				res(i, 0) += exp + exdif * fval; // exchange term - eq 1 NIST
			}

			return res;
		}

		static Eigen::MatrixXcd eexcDif(const Eigen::MatrixXcd& na, const Eigen::MatrixXcd& nb)
		{
			assert(na.cols() == 1 && nb.cols() == 1);

			int sz = static_cast<int>(na.rows());
			if (sz != nb.rows()) return {};

			static const double	X1d = 0.25 * pow(3. / (2. * M_PI), 2. * aThird);  // Exchange energy coefficient
			static const double X2d = pow(2., aThird);
			static const double fdd = 4. / (9. * (pow(2., aThird) - 1.));

			Eigen::MatrixXcd res(sz, 1);

			for (int i = 0; i < sz; ++i)
			{
				const double roa = na(i, 0).real();
				const double rob = nb(i, 0).real();
				const double n = roa + rob;
				if (n < 1E-18)
				{
					res(i, 0) = 0;
					continue;
				}

				const double rs = pow(3. / (fourM_PI * n), aThird);

				const double expd = X1d / rs;
				const double exfd = X2d * expd;

				const double zeta = (roa - rob) / n;
				const double zeta3 = zeta * zeta * zeta;
				const double zeta4 = zeta3 * zeta;

				const double fval = f(zeta);
				const double y = sqrt(rs);

				const double YP = y * (y + bP) + cP;
				const double difyP = y - y0P;
				const double ecp = F(y, difyP, AP, y0P, bP, cP, Y0P, YP); // B.5

				const double YF = y * (y + bF) + cF;
				const double difyF = y - y0F;
				const double ecf = F(y, difyF, AF, y0F, bF, cF, Y0F, YF); // B.5

				const double YA = y * (y + balpha) + calpha;
				const double difyA = y - y0alpha;
				const double eca = F(y, difyA, Aalpha, y0alpha, balpha, calpha, Y0alpha, YA); // B.5

				const double ecpd = ecDif(y, difyP, AP, y0P, bP, cP, Y0P, YP);
				const double ecfd = ecDif(y, difyF, AF, y0F, bF, cF, Y0F, YF);
				const double ecad = ecDif(y, difyA, Aalpha, y0alpha, balpha, calpha, Y0alpha, YA);

				const double deltaecfp = ecf - ecp; // eq 10 NIST
				const double beta = fdd * deltaecfp / eca - 1.; // eq. 9 NIST
				const double opbz4 = 1 + beta * zeta4;
				const double interp = fval / fdd * opbz4; // eq 8 NIST without alphac
				const double deltaec = eca * interp; // eq. 8 NIST

				const double betad = fdd / eca * (ecfd - ecpd - ecad * deltaecfp / eca);
				const double interpd = fval / fdd * zeta4 * betad;

				const double deriv = aThird * (ecpd // paramagnetic part
					+ ecad * interp + eca * interpd);

				res(i, 0) = expd + (exfd - expd) * fval // exchange term
					// correlation term:
					+ deriv;
			}

			return res;
		}
	};

}