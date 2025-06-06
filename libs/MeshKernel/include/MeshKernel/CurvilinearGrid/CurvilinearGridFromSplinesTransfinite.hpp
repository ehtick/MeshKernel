//---- GPL ---------------------------------------------------------------------
//
// Copyright (C)  Stichting Deltares, 2011-2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// contact: delft3d.support@deltares.nl
// Stichting Deltares
// P.O. Box 177
// 2600 MH Delft, The Netherlands
//
// All indications and logos of, and references to, "Delft3D" and "Deltares"
// are registered trademarks of Stichting Deltares, and remain the property of
// Stichting Deltares. All rights reserved.
//
//------------------------------------------------------------------------------

#pragma once

#include <memory>
#include <vector>

#include <MeshKernel/Constants.hpp>
#include <MeshKernel/Parameters.hpp>
#include <MeshKernel/Point.hpp>
#include <MeshKernel/Utilities/LinearAlgebra.hpp>

namespace meshkernel
{
    class CurvilinearGrid;
    class Splines;

    /// @brief A class used to create a curvilinear grid from 4 splines in blocks
    ///
    /// Usually CurvilinearGridFromSplines should be preferred.
    class CurvilinearGridFromSplinesTransfinite
    {
    public:
        /// @brief Ctor with splines and parameters
        /// @returns
        CurvilinearGridFromSplinesTransfinite(std::shared_ptr<Splines> splines, const CurvilinearParameters& curvilinearParameters);

        /// Computes the curvilinear grid from the splines using transfinite interpolation
        /// @param curvilinearGrid
        std::unique_ptr<CurvilinearGrid> Compute();

        std::shared_ptr<Splines> m_splines; ///< A pointer to spline

    private:
        /// @brief Characterise the splines
        ///
        /// finds intersections and orders the splines
        void CharacteriseSplines();

        /// @brief Compute spline intersections in n-direction
        void ComputeNDirectionIntersections();

        /// @brief Compute spline intersections in m-direction
        void ComputeMDirectionIntersections();

        /// @brief Compute spline intersection start and end points
        void ComputeSplineStartAndEnd(const UInt outerStart, const UInt outerEnd, const UInt innerStart, const UInt innerEnd);

        /// @brief Label each spline and its intersection.
        ///
        /// In which group does it lie (m or n), and spline crossing indices.
        void ClassifySplineIntersections();

        /// @brief Computes the adimensional intersections between splines.
        ///
        /// Also orders the m splines (the horizontal ones) before the n splines (the vertical ones)
        void ComputeInteractions();

        /// @brief Organise the splines by spline type (vertical or horizontal)
        void OrganiseSplines();

        /// @brief Order the splines such that their index increases in m or n direction
        /// @param[in] startFirst
        /// @param[in] endFirst
        /// @param[in] startSecond
        /// @param[in] endSecond
        /// @returns Boolean to indicate that procedure has to be repeated
        [[nodiscard]] bool OrderSplines(UInt startFirst,
                                        UInt endFirst,
                                        UInt startSecond,
                                        UInt endSecond);

        /// @brief Swap the columns of a two dimensional vector (MAKESR)
        /// @tparam T The input vector
        /// @param v
        /// @param firstColumn The first column
        /// @param secondColumn The second column
        template <typename T>
        void SwapColumns(std::vector<std::vector<T>>& v, UInt firstColumn, UInt secondColumn) const;

        /// Compute the distances following an exponential increase
        /// @param[in] factor
        /// @param[in] leftDistance
        /// @param[in] rightDistance
        /// @param[out] distances
        void ComputeExponentialDistances(double factor,
                                         double leftDistance,
                                         double rightDistance,
                                         std::vector<double>& distances) const;

        /// @brief Computes the distances along the spline where to generate the points
        /// @param[in] numIntersections
        /// @param[in] numPoints
        /// @param[in] numDiscretizations
        /// @param[in] intersectionDistances
        /// @param[out] distances
        void ComputeDiscretizations(UInt numIntersections,
                                    UInt numPoints,
                                    UInt numDiscretizations,
                                    const std::vector<double>& intersectionDistances,
                                    std::vector<double>& distances) const;

        /// @brief Fill side point arrays
        void FillInterpolationPlaneBlock(const lin_alg::Matrix<Point>& gridNodes,
                                         const UInt i,
                                         const UInt j,
                                         std::vector<Point>& bottomSide,
                                         std::vector<Point>& upperSide,
                                         std::vector<Point>& leftSide,
                                         std::vector<Point>& rightSide) const;

        /// @brief Assign interpolated points to the grid block
        void AssignInterpolatedNodes(const UInt i,
                                     const UInt j,
                                     const lin_alg::Matrix<Point>& interpolationResult,
                                     lin_alg::Matrix<Point>& gridNodes) const;

        std::vector<int> m_splineType;                                           ///< The spline types (1 horizontal, -1 vertical)
        std::vector<std::vector<double>> m_splineIntersectionRatios;             ///< For each spline, stores the intersections in terms of total spline length
        std::vector<std::vector<UInt>> m_splineGroupIndexAndFromToIntersections; ///< For each spline: position in m or n group, from and to spline crossing indices (MN12)
        UInt m_numMSplines = 0;                                                  ///< The index of the last m spline
        UInt m_numNSplines = 0;                                                  ///< The index of the last n spline

        UInt m_numM = 0; ///< Number of m columns
        UInt m_numN = 0; ///< Number of n rows
    };

} // namespace meshkernel
