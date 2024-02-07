#include "stdafx.h"
#include "ReciprocalSpace.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace DFT {


	ReciprocalSpaceCell::ReciprocalSpaceCell(const RealSpaceCell& realSpaceCell)
		: volume(0)
	{
		Init(realSpaceCell);
	}

	void ReciprocalSpaceCell::Init(const RealSpaceCell& realSpaceCell, double MaxFraction)
	{
		const Vector3D<double>& dim = realSpaceCell.GetSize();
		const double twopi = 2. * M_PI;
		m_dim = Vector3D(twopi / dim.X, twopi / dim.Y, twopi / dim.Z);
		volume = twopi * twopi * twopi / realSpaceCell.Volume();

		Indices.resize(realSpaceCell.Samples(), 1); 
		LatticeVectors.resize(realSpaceCell.Samples(), 1);
		LatticeVectorsSquaredMagnitude.resize(realSpaceCell.Samples(), 1);

		int i = 0;
		const Vector3D samples = realSpaceCell.GetSamples();

		for (int val1 = 0; val1 < samples.X; ++val1)
			for (int val2 = 0; val2 < samples.Y; ++val2)
				for (int val3 = 0; val3 < samples.Z; ++val3)
				{
					const Vector3D<int>& val = realSpaceCell.Indices(i);
					const Vector3D<int> offset(val.X > static_cast<int>(samples.X/2) ? samples.X : 0, val.Y > static_cast<int>(samples.Y/2) ? samples.Y : 0, val.Z > static_cast<int>(samples.Z/2) ? samples.Z : 0);
					const Vector3D<int> index(val - offset);
					const Vector3D<double> latticeVector(index.X / dim.X, index.Y / dim.Y, index.Z / dim.Z);

					Indices(i) =  index;
										
					LatticeVectors(i) = 2. * M_PI * latticeVector;
					LatticeVectorsSquaredMagnitude(i) = LatticeVectors(i) * LatticeVectors(i);
					
					++i;
				}

		Compress(realSpaceCell, MaxFraction);
	}


	void ReciprocalSpaceCell::Compress(const RealSpaceCell& realSpaceCell, double MaxFraction)
	{
		std::vector<int> edges;
		const Vector3D eS(realSpaceCell.GetSamples().X / 2. + 0.5, realSpaceCell.GetSamples().Y / 2. + 0.5, realSpaceCell.GetSamples().Z / 2. + 0.5);

		for (int i = 0; i < realSpaceCell.Indices.rows(); ++i)
		{
			const Vector3D dif(eS - realSpaceCell.Indices(i));

			if (abs(dif.X) < 1 || abs(dif.Y) < 1 || abs(dif.Z) < 1)
				edges.push_back(i);
		}

		if (!edges.empty())
		{
			double G2max = LatticeVectorsSquaredMagnitude(edges[0]);
			for (int i = 1; i < edges.size(); ++i)
				G2max = min(G2max, LatticeVectorsSquaredMagnitude(edges[i]));

			G2max *= MaxFraction * MaxFraction;

			for (int i = 0; i < LatticeVectorsSquaredMagnitude.rows(); ++i)
				if (LatticeVectorsSquaredMagnitude(i) < G2max)
					active.push_back(i);

			LatticeVectorsSquaredMagnitudeCompressed.resize(active.size(), 1);
			for (int i = 0; i < active.size(); ++i)
				LatticeVectorsSquaredMagnitudeCompressed(i) = LatticeVectorsSquaredMagnitude(active[i]);
		}
	}

}
