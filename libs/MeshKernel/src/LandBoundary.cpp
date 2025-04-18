//---- GPL ---------------------------------------------------------------------
//
// Copyright (C)  Stichting Deltares, 2011-2025.
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

#include "MeshKernel/LandBoundary.hpp"
#include "MeshKernel/Constants.hpp"
#include "MeshKernel/Operations.hpp"

meshkernel::LandBoundary::LandBoundary(const std::vector<Point>& landBoundary) : m_nodes(landBoundary) {}

void meshkernel::LandBoundary::FindNearestPoint(const Point& samplePoint,
                                                const Projection& projection,
                                                Point& nearestPoint,
                                                double& minimumDistance,
                                                UInt& segmentStartIndex,
                                                double& scaledDistanceToStart) const
{
    nearestPoint = samplePoint;
    segmentStartIndex = constants::missing::uintValue;
    scaledDistanceToStart = -1.0;

    minimumDistance = 9.0e33;

    if (!samplePoint.IsValid())
    {
        return;
    }

    for (UInt i = 0; i < m_nodes.size() - 1; ++i)
    {
        Point firstPoint = m_nodes[i];
        Point nextPoint = m_nodes[i + 1];

        auto [distance, linePoint, distanceFromFirstNode] = DistanceFromLine(samplePoint, firstPoint, nextPoint, projection);

        if (distance != constants::missing::doubleValue && distance < minimumDistance)
        {
            minimumDistance = distance;
            nearestPoint = linePoint;
            segmentStartIndex = i;
            scaledDistanceToStart = distanceFromFirstNode;
        }
    }
}

meshkernel::Point meshkernel::LandBoundary::FindNearestPoint(const Point& samplePoint,
                                                             const Projection& projection) const
{
    Point nearestPoint;
    [[maybe_unused]] double minimumDistance;
    [[maybe_unused]] UInt segmentStartIndex;
    [[maybe_unused]] double scaledDistanceToStart;
    FindNearestPoint(samplePoint, projection, nearestPoint, minimumDistance, segmentStartIndex, scaledDistanceToStart);

    return nearestPoint;
}

void meshkernel::LandBoundary::AddSegment(const Point& leftNode, const Point& rightNode)
{
    // Update nodes
    m_nodes.emplace_back(Point());
    m_nodes.emplace_back(leftNode);
    m_nodes.emplace_back(rightNode);
    m_nodes.emplace_back(Point());
}

std::vector<std::pair<meshkernel::UInt, meshkernel::UInt>> meshkernel::LandBoundary::FindPolylineIndices() const
{
    return FindIndices(m_nodes, 0, static_cast<UInt>(m_nodes.size()), constants::missing::doubleValue);
}

std::vector<bool> meshkernel::LandBoundary::GetNodeMask(const Polygons& polygons) const
{
    std::vector<bool> nodeMask(GetNumNodes(), false);

    for (UInt n = 0; n < m_nodes.size() - 1; n++)
    {
        if (!m_nodes[n].IsValid() || !m_nodes[n + 1].IsValid())
        {
            continue;
        }

        if (polygons.IsPointInPolygon(m_nodes[n], 0) ||
            polygons.IsPointInPolygon(m_nodes[n + 1], 0))
        {
            nodeMask[n] = true;
        }
    }

    return nodeMask;
}

meshkernel::Point meshkernel::LandBoundary::ClosestPoint(const Point& point, const size_t point1Index, const size_t point2Index, const Projection projection) const
{

    if (ComputeSquaredDistance(point, m_nodes[point1Index], projection) <= ComputeSquaredDistance(point, m_nodes[point2Index], projection))
    {
        return m_nodes[point1Index];
    }
    else
    {
        return m_nodes[point2Index];
    }
}

meshkernel::BoundingBox meshkernel::LandBoundary::GetBoundingBox(const size_t startIndex, const size_t endIndex) const
{
    return BoundingBox(m_nodes, startIndex, endIndex);
}

meshkernel::BoundingBox meshkernel::LandBoundary::GetBoundingBox() const
{
    return GetBoundingBox(0, m_nodes.size() - 1);
}
