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
#include <MeshKernel/CurvilinearGrid/CurvilinearGridFromPolygon.hpp>
#include <MeshKernel/Operations.hpp>
#include <MeshKernel/Polygon.hpp>

using meshkernel::CurvilinearGrid;
using meshkernel::CurvilinearGridFromPolygon;

CurvilinearGridFromPolygon::CurvilinearGridFromPolygon(const Polygon& polygon) : m_polygon(polygon) {}

void CurvilinearGridFromPolygon::ComputeNumberOfMNodes(const UInt firstNode,
                                                       const UInt secondNode,
                                                       const UInt numPolygonNodes,
                                                       int& direction,
                                                       UInt& numMNodes) const
{
    // get rid of size and orientation first part
    UInt diffForward;

    if (firstNode > secondNode)
    {
        diffForward = secondNode + numPolygonNodes - firstNode;
    }
    else
    {
        diffForward = secondNode - firstNode;
    }

    UInt diffBackward;
    if (secondNode > firstNode)
    {
        diffBackward = firstNode + numPolygonNodes - secondNode;
    }
    else
    {
        diffBackward = firstNode - secondNode;
    }

    if (diffForward <= diffBackward)
    {
        direction = 1;
        numMNodes = diffForward + 1;
    }
    else
    {
        direction = -1;
        numMNodes = diffBackward + 1;
    }
}

void CurvilinearGridFromPolygon::ComputeNumberOfNNodes(const UInt secondNode,
                                                       const UInt thirdNode,
                                                       const UInt numPolygonNodes,
                                                       const int direction,
                                                       UInt& numNNodes) const
{
    // get rid of size and orientation first part
    UInt diffForward;
    UInt diffBackward;

    // get rid of size and orientation second part
    if (secondNode > thirdNode)
    {
        diffForward = thirdNode + numPolygonNodes - secondNode;
    }
    else
    {
        diffForward = thirdNode - secondNode;
    }

    if (thirdNode > secondNode)
    {
        diffBackward = secondNode + numPolygonNodes - thirdNode;
    }
    else
    {
        diffBackward = secondNode - thirdNode;
    }

    if (direction == 1)
    {
        numNNodes = diffForward + 1;
    }
    else
    {
        numNNodes = diffBackward + 1;
    }
}

void CurvilinearGridFromPolygon::AssignPolygonPointsToSegment(UInt nodeIndex,
                                                              UInt numPointsSide,
                                                              int direction,
                                                              std::vector<Point>& sideToFill) const

{
    const std::vector<Point>& nodes = m_polygon.Nodes();
    const UInt numPolygonNodes = m_polygon.Size() - 1;

    for (UInt i = 0; i < numPointsSide; i++)
    {
        sideToFill[i] = nodes[nodeIndex];

        if (nodeIndex == 0 && direction == -1)
        {
            nodeIndex = nodeIndex + numPolygonNodes + direction;
        }
        else if (nodeIndex + direction > numPolygonNodes)
        {
            nodeIndex = nodeIndex + direction - numPolygonNodes;
        }
        else
        {
            nodeIndex = nodeIndex + direction;
        }
    }
}

std::unique_ptr<CurvilinearGrid> CurvilinearGridFromPolygon::Compute(UInt firstNode,
                                                                     UInt secondNode,
                                                                     UInt thirdNode,
                                                                     bool useFourthSide) const
{
    if (m_polygon.Size() < 4)
    {
        throw ConstraintError("The polygon does not contain sufficient nodes: count = {}", m_polygon.Size());
    }

    const auto areNodesValid = firstNode != secondNode &&
                               secondNode != thirdNode &&
                               firstNode != thirdNode;

    if (!areNodesValid)
    {
        throw ConstraintError("Invalid node selection, duplicate values found: first = {}, second = {}, third = {}",
                              firstNode, secondNode, thirdNode);
    }

    // for the current polygon find the number of nodes
    UInt end = m_polygon.Size() - 1;

    // This does not include the last, closing, node of the polygon
    const UInt numPolygonNodes = end;

    int direction;
    UInt numMNodes;
    UInt numNNodes;

    ComputeNumberOfMNodes(firstNode, secondNode, numPolygonNodes, direction, numMNodes);
    ComputeNumberOfNNodes(secondNode, thirdNode, numPolygonNodes, direction, numNNodes);

    // get the fourth node
    auto fourthNode = thirdNode + direction * (numMNodes - 1);

    if (fourthNode >= numPolygonNodes)
    {
        fourthNode -= numPolygonNodes;
    }

    UInt numRequiredPoints;
    if (useFourthSide)
    {
        numRequiredPoints = 2 * (numMNodes - 1) + 2 * (numNNodes - 1);
    }
    else
    {
        numRequiredPoints = 1 + 2 * (numMNodes - 1) + (numNNodes - 1);
    }

    if (numRequiredPoints > numPolygonNodes)
    {
        throw std::invalid_argument("CurvilinearGridFromPolygon::CurvilinearGridFromPolygon: The polygon does not contain enough nodes to compute the curvilinear grid.");
    }

    const auto maximumNumberOfNodes = std::max(numNNodes, numMNodes);
    std::vector<Point> sideOne(maximumNumberOfNodes, {constants::missing::doubleValue, constants::missing::doubleValue});
    std::vector<Point> sideTwo(maximumNumberOfNodes, {constants::missing::doubleValue, constants::missing::doubleValue});
    std::vector<Point> sideThree(maximumNumberOfNodes, {constants::missing::doubleValue, constants::missing::doubleValue});
    std::vector<Point> sideFour(maximumNumberOfNodes, {constants::missing::doubleValue, constants::missing::doubleValue});

    if (useFourthSide)
    {
        AssignPolygonPointsToSegment(firstNode, numNNodes, -direction, sideOne);
    }
    else
    {
        // Interpolate fourth side
        for (UInt i = 0; i < numNNodes; i++)
        {
            const double fac = static_cast<double>(i) / static_cast<double>(numNNodes - 1);
            sideOne[i] = m_polygon.Node(firstNode) * (1.0 - fac) +
                         m_polygon.Node(fourthNode) * fac;
        }
    }

    AssignPolygonPointsToSegment(secondNode, numNNodes, direction, sideTwo);
    AssignPolygonPointsToSegment(firstNode, numMNodes, direction, sideThree);
    AssignPolygonPointsToSegment(fourthNode, numMNodes, -direction, sideFour);

    Projection const polygonProjection = m_polygon.GetProjection();

    const auto result = DiscretizeTransfinite(sideOne,
                                              sideTwo,
                                              sideThree,
                                              sideFour,
                                              polygonProjection,
                                              numNNodes - 1,
                                              numMNodes - 1);

    return std::make_unique<CurvilinearGrid>(result, polygonProjection);
}

std::unique_ptr<CurvilinearGrid> CurvilinearGridFromPolygon::Compute(UInt firstNode,
                                                                     UInt secondNode,
                                                                     UInt thirdNode) const
{
    if (m_polygon.Size() < 4)
    {
        throw ConstraintError("The polygon does not contain sufficient nodes: count = {}", m_polygon.Size());
    }

    const auto areNodesValid = firstNode != secondNode && secondNode != thirdNode && firstNode != thirdNode;

    if (!areNodesValid)
    {
        throw std::invalid_argument("CurvilinearGridFromPolygon::Compute: Invalid nodes.");
    }

    // for the current polygon find the number of nodes
    UInt start = 0;
    UInt end = m_polygon.Size() - 1;

    // This does not include the last, closing, node of the polygon
    const auto numPolygonNodes = end - start;

    // get rid of size and orientation first part
    UInt numPointsFirstSide;

    if (firstNode > secondNode)
    {
        numPointsFirstSide = secondNode + numPolygonNodes - firstNode;
    }
    else
    {
        numPointsFirstSide = secondNode - firstNode;
    }

    UInt numPointsSecondSide;
    if (secondNode > thirdNode)
    {
        numPointsSecondSide = thirdNode + numPolygonNodes - secondNode;
    }
    else
    {
        numPointsSecondSide = thirdNode - secondNode;
    }

    const UInt numPointsThirdSide = numPointsFirstSide + numPointsSecondSide > numPolygonNodes ? 0u : numPolygonNodes - (numPointsFirstSide + numPointsSecondSide);
    const auto blockSize = static_cast<UInt>(static_cast<double>(numPointsFirstSide + numPointsSecondSide + numPointsThirdSide) * 0.5);

    if (numPointsFirstSide >= blockSize)
    {
        throw std::invalid_argument("CurvilinearGridFromPolygon::Compute "
                                    "The number of points on the first side is " +
                                    std::to_string(numPointsFirstSide) + " which cannot be larger or equal to the block size " + std::to_string(blockSize));
    }

    if (numPointsSecondSide >= blockSize)
    {
        throw std::invalid_argument("CurvilinearGridFromPolygon::Compute "
                                    "The number of points on the second side is " +
                                    std::to_string(numPointsSecondSide) + " which cannot be larger or equal to the block size " + std::to_string(blockSize));
    }

    if (numPointsThirdSide >= blockSize)
    {
        throw std::invalid_argument("CurvilinearGridFromPolygon::Compute "
                                    "The number of points on the third side is " +
                                    std::to_string(numPointsThirdSide) + " which cannot be larger or equal to the block size " + std::to_string(blockSize));
    }

    const auto n1 = blockSize - numPointsThirdSide;
    const auto n2 = blockSize - numPointsSecondSide;
    const auto n3 = blockSize - numPointsFirstSide;

    // compute the midpoint

    UInt firstSideMiddlePoint = firstNode + n1;
    if (firstSideMiddlePoint >= numPolygonNodes)
    {
        firstSideMiddlePoint = firstSideMiddlePoint - numPolygonNodes;
    }
    UInt secondSideMiddlePoint = secondNode + n3;
    if (secondSideMiddlePoint >= numPolygonNodes)
    {
        secondSideMiddlePoint = secondSideMiddlePoint - numPolygonNodes;
    }
    UInt thirdSideMiddlePoint = thirdNode + n2;
    if (thirdSideMiddlePoint >= numPolygonNodes)
    {
        thirdSideMiddlePoint = thirdSideMiddlePoint - numPolygonNodes;
    }

    // set dimensions of blocks
    std::vector<UInt> numN{n1, n3, n2};
    std::vector<UInt> numM{n3, n2, n1};

    // set pointers of block corners
    std::vector<UInt> cornerPoints{firstNode, secondNode, thirdNode};
    std::vector<UInt> iLeft{thirdSideMiddlePoint, firstSideMiddlePoint, secondSideMiddlePoint};
    std::vector<UInt> iRight{firstSideMiddlePoint, secondSideMiddlePoint, thirdSideMiddlePoint};

    // compute triangle middle point
    const auto xia = static_cast<double>(n1) / static_cast<double>(numPointsFirstSide);
    const auto xib = static_cast<double>(n2) / static_cast<double>(numPointsSecondSide);
    const auto xic = static_cast<double>(n3) / static_cast<double>(numPointsThirdSide);

    auto const& polygonNodes = m_polygon.Nodes();

    const auto triangleCenter = ((polygonNodes[firstNode] * (1.0 - xia) + polygonNodes[secondNode] * xia) * xic + polygonNodes[thirdNode] * (1.0 - xic) +
                                 (polygonNodes[secondNode] * (1.0 - xib) + polygonNodes[thirdNode] * xib) * xia + polygonNodes[firstNode] * (1.0 - xia) +
                                 (polygonNodes[thirdNode] * (1.0 - xic) + polygonNodes[firstNode] * xic) * xib + polygonNodes[secondNode] * (1.0 - xib)) *
                                constants::numeric::oneThird;

    const auto maxM = *std::ranges::max_element(numM);
    const auto maxN = *std::ranges::max_element(numN);
    const auto maximumNumberOfNodes = std::max(maxM, maxN) + 1;
    std::vector<Point> sideOne(maximumNumberOfNodes);
    std::vector<Point> sideTwo(maximumNumberOfNodes);
    std::vector<Point> sideThree(maximumNumberOfNodes);
    std::vector<Point> sideFour(maximumNumberOfNodes);

    lin_alg::Matrix<Point> gridNodes(n1 + n3 + 1, n2 + n3 + 1);

    Projection const polygonProjection = m_polygon.GetProjection();

    for (UInt t = 0; t < constants::geometric::numNodesInTriangle; ++t)
    {
        std::ranges::fill(sideOne, Point());
        std::ranges::fill(sideTwo, Point());
        std::ranges::fill(sideThree, Point());
        std::ranges::fill(sideFour, Point());

        // backward
        auto cornerIndex = cornerPoints[t];
        for (UInt i = 0; i < numM[t] + 1; ++i)
        {
            sideOne[i] = polygonNodes[cornerIndex];
            if (cornerIndex == 0 || cornerIndex < start)
            {
                cornerIndex = cornerIndex + numPolygonNodes - 1;
            }
            else if (cornerIndex > end)
            {
                cornerIndex = cornerIndex - numPolygonNodes - 1;
            }
            else
            {
                cornerIndex -= 1;
            }
        }

        // forward
        cornerIndex = cornerPoints[t];
        for (UInt i = 0; i < numN[t] + 1; ++i)
        {
            sideThree[i] = polygonNodes[cornerIndex];
            cornerIndex += 1;
            if (cornerIndex < start)
            {
                cornerIndex = cornerIndex + numPolygonNodes;
            }
            if (cornerIndex > end)
            {
                cornerIndex = cornerIndex - numPolygonNodes;
            }
        }

        // fill side four
        for (UInt i = 0; i < numN[t] + 1; ++i)
        {
            double localXia = static_cast<double>(i) / static_cast<double>(numN[t]);
            sideFour[i] = polygonNodes[iLeft[t]] * (1.0 - localXia) + triangleCenter * localXia;
        }

        // fill side two
        for (UInt i = 0; i < numM[t] + 1; ++i)
        {
            double localXia = static_cast<double>(i) / static_cast<double>(numM[t]);
            sideTwo[i] = polygonNodes[iRight[t]] * (1.0 - localXia) + triangleCenter * localXia;
        }
        const auto result = DiscretizeTransfinite(sideOne,
                                                  sideTwo,
                                                  sideThree,
                                                  sideFour,
                                                  polygonProjection,
                                                  numM[t],
                                                  numN[t]);
        // add to grid
        if (t == 0)
        {
            // gridNodes = result;
            for (UInt i = 0; i < result.rows(); ++i)
            {
                for (UInt j = 0; j < result.cols(); ++j)
                {
                    gridNodes(i, j) = result(i, j);
                }
            }
        }
        if (t == 1)
        {
            for (UInt i = 0; i < result.rows(); ++i)
            {
                for (UInt j = 0; j < result.cols(); ++j)
                {
                    const auto iIndex = n1 + n3 - i;
                    const auto jIndex = n2 + n3 - j;
                    gridNodes(iIndex, jIndex) = result(i, j);
                }
            }
        }
        if (t == 2)
        {
            for (UInt i = 0; i < result.cols(); ++i)
            {
                for (UInt j = 0; j < result.rows(); ++j)
                {
                    const auto jIndex = n2 + n3 - j;
                    gridNodes(i, jIndex) = result(j, i);
                }
            }
        }
    }

    return std::make_unique<CurvilinearGrid>(gridNodes, polygonProjection);
}
