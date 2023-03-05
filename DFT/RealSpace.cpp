#include "stdafx.h"
#include "RealSpace.h"

namespace DFT {

	RealSpaceCell::RealSpaceCell(double dim1, double dim2, double dim3, unsigned int samples1, unsigned int samples2, unsigned int samples3)
		: m_dim(dim1, dim2, dim3), m_samples(samples1, samples2, samples3), 
		volume(dim1*dim2*dim3), samples(samples1*samples2*samples3), invSamples(1. / samples), sampleVolume(volume * invSamples),
		Indices(samples, 1), SamplePoints(samples, 1)
	{
		Vector3D<double> spacing(m_dim.X / m_samples.X, m_dim.Y / m_samples.Y, m_dim.Z / m_samples.Z);

		unsigned int i = 0;

		for (unsigned int val1 = 0; val1 < m_samples.X; ++val1)
			for (unsigned int val2 = 0; val2 < m_samples.Y; ++val2)
				for (unsigned int val3 = 0; val3 < m_samples.Z; ++val3)
				{
					const Vector3D<unsigned int> val = Vector3D<unsigned int>(val1, val2, val3);
					Indices(i) = val;
					SamplePoints(i++) = Vector3D<double>(spacing.X * val.X, spacing.Y * val.Y, spacing.Z * val.Z);
				}
	}


	RealSpaceCell::~RealSpaceCell()
	{
	}

}