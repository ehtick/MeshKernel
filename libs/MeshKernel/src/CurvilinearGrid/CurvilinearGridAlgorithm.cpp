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

#include <MeshKernel/CurvilinearGrid/CurvilinearGrid.hpp>
#include <MeshKernel/CurvilinearGrid/CurvilinearGridAlgorithm.hpp>
#include <MeshKernel/CurvilinearGrid/CurvilinearGridLine.hpp>
#include <MeshKernel/Exceptions.hpp>

using meshkernel::CurvilinearGrid;
using meshkernel::CurvilinearGridAlgorithm;
using meshkernel::CurvilinearGridLine;

CurvilinearGridAlgorithm::CurvilinearGridAlgorithm(CurvilinearGrid& grid) : m_grid(grid),
                                                                            m_isGridNodeFrozen(m_grid.NumN(), m_grid.NumM())
{
}

void CurvilinearGridAlgorithm::SetBlock(Point const& firstCornerPoint, Point const& secondCornerPoint)
{
    // Get the m and n indices from the point coordinates
    auto const [lowerLeft, upperRight] = m_grid.ComputeBlockFromCornerPoints(firstCornerPoint, secondCornerPoint);

    // Coinciding corner nodes, no valid area, nothing to do
    if (lowerLeft == upperRight)
    {
        throw std::invalid_argument("CurvilinearGridSmoothing::SetBlock coinciding corner nodes, no valid area to smooth");
    }

    m_lowerLeft = lowerLeft;
    m_upperRight = upperRight;
}

void CurvilinearGridAlgorithm::SetBlock(CurvilinearGridNodeIndices const& firstCornerPoint, CurvilinearGridNodeIndices const& secondCornerPoint)
{
    // Coinciding corner nodes, no valid area, nothing to do
    if (firstCornerPoint == secondCornerPoint)
    {
        throw std::invalid_argument("CurvilinearGridSmoothing::SetBlock coinciding corner nodes, no valid area to smooth");
    }

    m_lowerLeft = firstCornerPoint;
    m_upperRight = secondCornerPoint;
}

void CurvilinearGridAlgorithm::SetLine(Point const& firstPoint, Point const& secondPoint)
{
    // The selected nodes must be on the vertical or horizontal line
    const auto [newLineLowerLeft, newLineUpperRight] = m_grid.ComputeBlockFromCornerPoints(firstPoint, secondPoint);

    // Coinciding nodes, no valid line, nothing to do
    if (newLineLowerLeft == newLineUpperRight)
    {
        throw AlgorithmError("The start and the end points of the line are coinciding");
    }

    // The points of the frozen line, must be on the same grid-line
    if (!newLineLowerLeft.IsOnTheSameGridLine(newLineUpperRight))
    {
        throw AlgorithmError("The nodes do not define a grid line");
    }

    // Store a new grid line
    m_lines.emplace_back(newLineLowerLeft, newLineUpperRight);
}

void CurvilinearGridAlgorithm::ComputeFrozenNodes()
{
    m_isGridNodeFrozen.fill(false);
    for (auto const& frozenLine : m_lines)
    {

        for (auto n = frozenLine.m_startNode.m_n; n <= frozenLine.m_endNode.m_n; ++n)
        {
            for (auto m = frozenLine.m_startNode.m_m; m <= frozenLine.m_endNode.m_m; ++m)
            {
                m_isGridNodeFrozen(n, m) = true;
            }
        }
    }
}
