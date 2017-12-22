#pragma once
class Options
{
public:
	Options();
	~Options();

	void Load();
	void Save();


	double dim1;
	double dim2;
	double dim3;

	unsigned int samples1;
	unsigned int samples2;
	unsigned int samples3;

	int Ns;

	unsigned int SteepestDescentSteps;
	unsigned int MinFindSteps;
	
	int algorithm; // 0 - line minimization, 1 - preconditioned line minimization, 2 - Polak Ribiere conjugate gradient, 3 - Hestenes Stiefel conj grad, 4 - Fletcher Reeves conj grad

	int harmonic; // 1 - harmonic potential, 0 - linear

	int numThreadsFFT;

protected:
	static double GetDouble(LPCTSTR param, double defval);
};

