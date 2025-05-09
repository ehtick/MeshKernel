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

#include "MeshKernel/Definitions.hpp"

#include <cmath>
#include <limits>
#include <math.h>

#if defined(__linux__) || defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#include <Eigen/Core>

#if defined(__linux__) || defined(__APPLE__)
#pragma GCC diagnostic pop
#endif

namespace meshkernel
{

    namespace constants
    {
        // missing values
        namespace missing
        {
            constexpr double innerOuterSeparator = -998.0;                                     ///< Double value used to separate the inner part of a polygon from its outer part
            constexpr double doubleValue = -999.0;                                             ///< Double value used as missing value
            constexpr int intValue = -999;                                                     ///< Integer value used as missing value
            constexpr UInt uintValue = std::numeric_limits<UInt>::max();                       ///< missing value used for invalid indices
            constexpr Eigen::Index EigenIndexValue = std::numeric_limits<Eigen::Index>::max(); ///< missing value used for invalid eigen indices

        } // namespace missing

        // often used values
        namespace numeric
        {
            static double const squareRootOfThree = std::sqrt(3.0); ///< The result of sqrt(3)
            constexpr double oneThird = 1.0 / 3.0;                  ///< The result of 1 / 3
            constexpr int defaultSnappingIterations = 5;            ///< Default number of iterations to take in the spline snapping algorithm

        } // namespace numeric

        // unit conversion constants
        namespace conversion
        {
            constexpr double degToRad = M_PI / 180.0;   ///< Conversion factor from degrees to radians(pi / 180)
            constexpr double radToDeg = 1.0 / degToRad; ///< Conversion factor from radians to degrees(180 / pi)

        } // namespace conversion

        // geometric constants
        namespace geometric
        {
            constexpr double earth_radius = 6378137.0;                                      ///< Earth radius(m)
            constexpr double inverse_earth_radius = 1.0 / earth_radius;                     ///< One over constants::geometric::earth_radius(m-1);
            constexpr double absLatitudeAtPoles = 0.0001;                                   ///< Pole tolerance in degrees
            constexpr double refinementTolerance = 1.0e-2;                                  ///< Relative size of refinement.
            constexpr UInt numNodesInQuadrilateral = 4;                                     ///< Number of nodes in a quadrilateral
            constexpr UInt numNodesInTriangle = 3;                                          ///< Number of nodes in a triangle
            constexpr UInt numNodesInPentagon = 5;                                          ///< Number of nodes in a pentagon
            constexpr UInt numNodesInHexagon = 6;                                           ///< Number of nodes in a hexagon
            constexpr UInt maximumNumberOfEdgesPerNode = 16;                                ///< Maximum number of edges per node
            constexpr UInt maximumNumberOfEdgesPerFace = 6;                                 ///< Maximum number of edges per face
            constexpr UInt maximumNumberOfNodesPerFace = 6;                                 ///< Maximum number of nodes per face
            constexpr UInt maximumNumberOfConnectedNodes = maximumNumberOfEdgesPerNode * 4; ///< Maximum number of connected nodes

        } // namespace geometric

        namespace physical
        {
            constexpr double gravity = 9.80665;                    ///< Gravitational acceleration on earth (m/s^2)
            static double const sqrt_gravity = std::sqrt(gravity); ///< Square root of gravitational acceleration on earth

        } // namespace physical

    } // namespace constants
} // namespace meshkernel
