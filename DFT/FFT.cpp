#include "stdafx.h"
#include "FFT.h"

namespace Fourier {

	FFT::FFT(int numThreads)
	{
		fftw_init_threads();
		SetNumThreads(numThreads);
	}


	FFT::~FFT()
	{
	}


	void FFT::SetNumThreads(int numThreads)
	{
		Clear();
		fftw_plan_with_nthreads(numThreads);
	}

}