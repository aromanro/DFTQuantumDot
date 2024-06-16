#pragma once

#include "RealSpace.h"

namespace DFT {


	class ReciprocalSpaceCell
	{
	public:
		ReciprocalSpaceCell(const RealSpaceCell& realSpaceCell);

		void Init(const RealSpaceCell& realSpaceCell, double MaxFraction = 0.5);

		const Vector3D<double>& GetSize() const { return m_dim; }

		double Volume() const { return volume; }

		Eigen::Matrix<Vector3D<int>, Eigen::Dynamic, 1> Indices;
		Eigen::Matrix<Vector3D<double>, Eigen::Dynamic, 1> LatticeVectors;
		Eigen::Matrix<double, Eigen::Dynamic, 1> LatticeVectorsSquaredMagnitude;

		// active list and 'compressed' G2 containing the active values
		std::vector<int> active;
		Eigen::Matrix<double, Eigen::Dynamic, 1> LatticeVectorsSquaredMagnitudeCompressed;

	private:
		void Compress(const RealSpaceCell& realSpaceCell, double MaxFraction);

		Vector3D<double> m_dim;

		double volume;
	};

}