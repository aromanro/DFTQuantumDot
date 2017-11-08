#include "stdafx.h"
#include "Options.h"

#include "DFT.h"

Options::Options()
	: dim1(6), dim2(6), dim3(6),
	samples1(32), samples2(32), samples3(32),
	Ns(10),
	SteepestDescentSteps(40), MinFindSteps(100), algorithm(4),
	harmonic(1)
{
}


Options::~Options()
{
}

double Options::GetDouble(LPCTSTR param, double defval)
{
	double val = defval;

	UINT sz = 0;
	LPBYTE buf = NULL;

	if (theApp.GetProfileBinary(L"options", param, &buf, &sz))
	{
		if (sizeof(double) == sz) val = *((double*)buf);
		delete[] buf;
	}

	return val;
}

void Options::Load()
{
	dim1 = GetDouble(L"dim1", 6.);
	dim2 = GetDouble(L"dim2", 6.);
	dim3 = GetDouble(L"dim3", 6.);

	samples1 = theApp.GetProfileInt(L"options", L"samples1", 32);
	samples2 = theApp.GetProfileInt(L"options", L"samples2", 32);
	samples3 = theApp.GetProfileInt(L"options", L"samples3", 32);

	Ns = theApp.GetProfileInt(L"options", L"Ns", 10);

	SteepestDescentSteps = theApp.GetProfileInt(L"options", L"SteepestDescentSteps", 40);
	MinFindSteps = theApp.GetProfileInt(L"options", L"MinFindSteps", 100);
	algorithm = theApp.GetProfileInt(L"options", L"algorithm", 4);

	harmonic = theApp.GetProfileInt(L"options", L"harmonic", 1);
}

void Options::Save()
{
	theApp.WriteProfileBinary(L"options", L"dim1", (LPBYTE)&dim1, sizeof(double));
	theApp.WriteProfileBinary(L"options", L"dim2", (LPBYTE)&dim2, sizeof(double));
	theApp.WriteProfileBinary(L"options", L"dim3", (LPBYTE)&dim3, sizeof(double));

	theApp.WriteProfileInt(L"options", L"samples1", (int)samples1);
	theApp.WriteProfileInt(L"options", L"samples2", (int)samples2);
	theApp.WriteProfileInt(L"options", L"samples3", (int)samples3);

	theApp.WriteProfileInt(L"options", L"Ns", Ns);

	theApp.WriteProfileInt(L"options", L"SteepestDescentSteps", (int)SteepestDescentSteps);
	theApp.WriteProfileInt(L"options", L"MinFindSteps", (int)MinFindSteps);
	theApp.WriteProfileInt(L"options", L"algorithm", algorithm);

	theApp.WriteProfileInt(L"options", L"harmonic", harmonic);
}
