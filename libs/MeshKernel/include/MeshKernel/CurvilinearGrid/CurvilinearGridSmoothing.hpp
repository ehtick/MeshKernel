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

#include "MeshKernel/CurvilinearGrid/CurvilinearGrid.hpp"
#include "MeshKernel/CurvilinearGrid/CurvilinearGridAlgorithm.hpp"
#include "MeshKernel/UndoActions/UndoAction.hpp"

namespace meshkernel
{

    /// @brief A class implementing the curvilinear grid smoothing algorithm
    class CurvilinearGridSmoothing : public CurvilinearGridAlgorithm
    {
    public:
        /// @brief Class constructor
        /// @param[in] grid                        The input curvilinear grid
        /// @param[in] smoothingIterations         The number of smoothing iterations to perform
        CurvilinearGridSmoothing(CurvilinearGrid& grid, UInt smoothingIterations);

        /// @brief Compute curvilinear grid block smoothing (modifies the m_grid nodal values)
        [[nodiscard]] UndoActionPtr Compute() override;

        /// @brief Compute curvilinear grid line smoothing. The algorithm smooths the grid along the direction specified by the line.
        /// The line must be an m or n grid line of the curvilinear grid.
        /// @param[in] firstPoint The first point of the grid line
        /// @param[in] secondPoint The second point of the grid line
        /// @return The smoothed grid
        std::unique_ptr<CurvilinearGrid> ComputeDirectional(const Point& firstPoint, const Point& secondPoint);

    private:
        /// @brief Solve one iteration of block smoothing
        void Solve();

        /// @brief Solve one iteration of directional smoothing
        void SolveDirectional(const CurvilinearGridLine& gridLine);

        /// @brief Projects a point on the closest grid boundary
        /// @param[in] point The point to project
        /// @param[in] n The current n coordinate on the boundary of the curvilinear grid
        /// @param[in] m The current m coordinate on the boundary of the curvilinear grid
        void ProjectPointOnClosestGridBoundary(Point const& point, UInt n, UInt m);

        /// @brief Compute the edge lengths around a node (n,m) for n or m-grid line
        std::tuple<Point, Point> ComputeGridDelta(const UInt n, const UInt m, const CurvilinearGridLine& gridLine) const;

        /// @brief Compute a new smoothed grid point
        Point ComputeSmoothedGridNode(const UInt n,
                                      const UInt m,
                                      const CurvilinearGridLine& gridLine,
                                      const double firstLengthSquared,
                                      const double secondLengthSquared) const;

        UInt m_smoothingIterations;              ///< The orthogonalization parameters
        lin_alg::Matrix<Point> m_gridNodesCache; ///< A cache for storing current iteration node positions
    };
} // namespace meshkernel
