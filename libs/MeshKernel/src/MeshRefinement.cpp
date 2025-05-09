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

#include "MeshKernel/Utilities/RTreeFactory.hpp"

#include <MeshKernel/AveragingInterpolation.hpp>
#include <MeshKernel/Entities.hpp>
#include <MeshKernel/Exceptions.hpp>
#include <MeshKernel/Mesh2D.hpp>
#include <MeshKernel/MeshEdgeLength.hpp>
#include <MeshKernel/MeshRefinement.hpp>
#include <MeshKernel/Operations.hpp>
#include <MeshKernel/UndoActions/CompoundUndoAction.hpp>

using meshkernel::Mesh2D;
using meshkernel::MeshRefinement;

MeshRefinement::MeshRefinement(Mesh2D& mesh,
                               std::unique_ptr<MeshInterpolation> interpolant,
                               const MeshRefinementParameters& meshRefinementParameters) : m_samplesRTree(RTreeFactory::Create(mesh.m_projection)),
                                                                                           m_mesh(mesh),
                                                                                           m_interpolant(std::move(interpolant))
{
    CheckMeshRefinementParameters(meshRefinementParameters);
    m_isRefinementBasedOnSamples = true;
    m_meshRefinementParameters = meshRefinementParameters;
    m_refinementType = static_cast<RefinementType>(m_meshRefinementParameters.refinement_type);
}

MeshRefinement::MeshRefinement(Mesh2D& mesh,
                               std::unique_ptr<MeshInterpolation> interpolant,
                               const MeshRefinementParameters& meshRefinementParameters,
                               bool useNodalRefinement) : m_samplesRTree(RTreeFactory::Create(mesh.m_projection)),
                                                          m_mesh(mesh),
                                                          m_interpolant(std::move(interpolant)),
                                                          m_useNodalRefinement(useNodalRefinement)
{
    CheckMeshRefinementParameters(meshRefinementParameters);
    m_isRefinementBasedOnSamples = true;
    m_meshRefinementParameters = meshRefinementParameters;
    m_refinementType = static_cast<RefinementType>(m_meshRefinementParameters.refinement_type);
}

MeshRefinement::MeshRefinement(Mesh2D& mesh,
                               const Polygons& polygon,
                               const MeshRefinementParameters& meshRefinementParameters) : m_samplesRTree(RTreeFactory::Create(mesh.m_projection)),
                                                                                           m_mesh(mesh),
                                                                                           m_polygons(polygon)
{
    CheckMeshRefinementParameters(meshRefinementParameters);
    m_meshRefinementParameters = meshRefinementParameters;
}

void MeshRefinement::UpdateFaceMask(const int level)
{
    if (level == 0)
    {
        // if one face node is in polygon enable face refinement
        for (UInt f = 0; f < m_mesh.GetNumFaces(); ++f)
        {
            bool activeNodeFound = false;
            for (UInt n = 0; n < m_mesh.GetNumFaceEdges(f); ++n)
            {
                const auto nodeIndex = m_mesh.m_facesNodes[f][n];
                if (m_nodeMask[nodeIndex] != 0 && m_nodeMask[nodeIndex] != -2)
                {
                    activeNodeFound = true;
                    break;
                }
            }
            if (!activeNodeFound)
            {
                m_faceMask[f] = 0;
            }
        }
    }
    if (level > 0)
    {
        // if one face node is not in polygon disable refinement
        for (UInt f = 0; f < m_mesh.GetNumFaces(); f++)
        {
            for (UInt n = 0; n < m_mesh.GetNumFaceEdges(f); n++)
            {
                const auto nodeIndex = m_mesh.m_facesNodes[f][n];
                if (m_nodeMask[nodeIndex] != 1)
                {
                    m_faceMask[f] = 0;
                    break;
                }
            }
        }
    }
}

std::unique_ptr<meshkernel::UndoAction> MeshRefinement::Compute()
{
    std::unique_ptr<meshkernel::CompoundUndoAction> refinementAction = CompoundUndoAction::Create();

    // administrate mesh once more
    m_mesh.Administrate(refinementAction.get());

    // all faces and edges refined
    m_faceMask.resize(m_mesh.GetNumFaces(), 1);
    m_edgeMask.resize(m_mesh.GetNumEdges(), -1);

    // get bounding box
    Point lowerLeft{constants::missing::doubleValue, constants::missing::doubleValue};
    Point upperRight{constants::missing::doubleValue, constants::missing::doubleValue};
    if (m_mesh.m_projection == Projection::spherical)
    {
        const auto boundingBox = BoundingBox(m_mesh.Nodes());
        lowerLeft = boundingBox.lowerLeft();
        upperRight = boundingBox.upperRight();
    }

    // select the nodes to refine
    if (!m_isRefinementBasedOnSamples && m_meshRefinementParameters.refine_intersected == 1)
    {
        const auto edgeMask = m_mesh.MaskEdgesOfFacesInPolygon(m_polygons, false, true);
        m_nodeMask = m_mesh.NodeMaskFromEdgeMask(edgeMask);
    }
    else
    {
        m_nodeMask = m_mesh.NodeMaskFromPolygon(m_polygons, true);
    }

    FindBrotherEdges();

    // set_initial_mask
    ComputeNodeMaskAtPolygonPerimeter();

    auto numFacesAfterRefinement = m_mesh.GetNumFaces();
    // reserve some extra capacity for the node mask
    m_nodeMask.reserve(m_nodeMask.size() * 2);
    for (auto level = 0; level < m_meshRefinementParameters.max_num_refinement_iterations; level++)
    {
        // Compute all edge lengths at once
        ComputeEdgeBelowMinSizeAfterRefinement();

        // computes the edge and face refinement mask from samples
        if (m_isRefinementBasedOnSamples)
        {
            ComputeRefinementMasksFromSamples();
        }
        else
        {
            ComputeRefinementMaskFromEdgeSize();
        }

        UpdateFaceMask(level);
        ComputeEdgesRefinementMask();
        ComputeIfFaceShouldBeSplit();

        auto numFacesToRefine = std::count_if(m_faceMask.begin(), m_faceMask.begin() + m_mesh.GetNumFaces(),
                                              [](const int maskValue)
                                              { return maskValue != 0; });

        if (numFacesToRefine == 0)
        {
            break;
        }
        numFacesAfterRefinement = numFacesAfterRefinement * 4;

        // spit the edges
        refinementAction->Add(RefineFacesBySplittingEdges());

        // TODO return action
        refinementAction->Add(m_mesh.OffsetSphericalCoordinates(lowerLeft.x, upperRight.x));

        m_mesh.Administrate(refinementAction.get());

        m_faceMask.resize(m_mesh.GetNumFaces());
        m_edgeMask.resize(m_mesh.GetNumEdges());

        FindBrotherEdges();
    }

    // remove isolated hanging nodes and connect if needed
    if (m_meshRefinementParameters.connect_hanging_nodes == 1)
    {
        refinementAction->Add(ConnectHangingNodes());
        m_mesh.Administrate(refinementAction.get());
    }

    return refinementAction;
}

void MeshRefinement::ComputeRefinementMaskFromEdgeSize()
{
    std::ranges::fill(m_edgeMask, 0);
    std::ranges::fill(m_faceMask, 0);
    for (UInt f = 0; f < m_mesh.GetNumFaces(); ++f)
    {
        for (UInt n = 0; n < m_mesh.GetNumFaceEdges(f); ++n)
        {
            const auto e = m_mesh.m_facesEdges[f][n];
            if (!m_isEdgeBelowMinSizeAfterRefinement[e])
            {
                m_edgeMask[e] = -1;
                m_faceMask[f] = 1;
            }
        }
    }
}

#if 0
// This can be removed
meshkernel::UInt MeshRefinement::DeleteIsolatedHangingnodes()
{
    UInt numRemovedIsolatedHangingNodes = 0;
    for (UInt e = 0; e < m_mesh.GetNumEdges(); ++e)
    {
        const auto brotherEdgeIndex = m_brotherEdges[e];
        if (brotherEdgeIndex == constants::missing::uintValue)
        {
            continue;
        }

        const auto commonNode = m_mesh.FindCommonNode(e, brotherEdgeIndex);
        if (commonNode == constants::missing::uintValue)
        {
            continue;
        }

        if (commonNode > 0 && m_mesh.GetNumNodesEdges(commonNode) == 2)
        {
            for (UInt f = 0; f < m_mesh.GetNumEdgesFaces(e); ++f)
            {
                const auto faceIndex = m_mesh.m_edgesFaces[e][f];

                if (faceIndex != m_mesh.m_edgesFaces[brotherEdgeIndex][0] &&
                    faceIndex != m_mesh.m_edgesFaces[brotherEdgeIndex][std::min(m_mesh.GetNumEdgesFaces(brotherEdgeIndex), static_cast<UInt>(1))])
                {
                    throw AlgorithmError("Algorithm error.");
                }

                UInt ee = 0;
                UInt nn = 0;
                for (UInt n = 0; n < m_mesh.GetNumFaceEdges(faceIndex); ++n)
                {
                    const auto edgeIndex = m_mesh.m_facesEdges[faceIndex][n];
                    if (edgeIndex != brotherEdgeIndex)
                    {
                        m_mesh.m_facesEdges[faceIndex][ee] = edgeIndex;
                        ee++;
                    }

                    const auto nodeIndex = m_mesh.m_facesEdges[faceIndex][n];
                    if (nodeIndex != commonNode)
                    {
                        m_mesh.m_facesNodes[faceIndex][nn] = nodeIndex;
                        nn++;
                    }
                }

                m_mesh.m_numFacesNodes[faceIndex] -= 1;

                if (m_mesh.m_numFacesNodes[faceIndex] != ee || m_mesh.m_numFacesNodes[faceIndex] != nn)
                {
                    throw AlgorithmError("Algorithm error.");
                }
            }

            const auto otherNodeIndex = OtherNodeOfEdge(m_mesh.GetEdge(brotherEdgeIndex), commonNode);

            // update lin admin
            if (m_mesh.GetEdge(e).first == commonNode)
            {
                m_mesh.SetEdge(e, {otherNodeIndex, m_mesh.GetEdge(e).second});
            }
            else
            {
                m_mesh.SetEdge(e, {m_mesh.GetEdge(e).first, otherNodeIndex});
            }

            // change node adm of other node
            for (UInt ee = 0; ee < m_mesh.GetNumNodesEdges(otherNodeIndex); ++ee)
            {
                if (m_mesh.m_nodesEdges[otherNodeIndex][ee] == brotherEdgeIndex)
                {
                    m_mesh.m_nodesEdges[otherNodeIndex][ee] = e;
                    break;
                }
            }

            // delete node
            numRemovedIsolatedHangingNodes->Add(m_mesh.DeleteNode(commonNode));
            numRemovedIsolatedHangingNodes->Add(m_mesh.DeleteEdge(brotherEdgeIndex));

            m_brotherEdges[brotherEdgeIndex] = constants::missing::uintValue;

            numRemovedIsolatedHangingNodes++;
        }
    }
    return numRemovedIsolatedHangingNodes;
}
#endif

void MeshRefinement::ConnectOneHangingNodeForQuadrilateral(const UInt numNonHangingNodes,
                                                           const std::vector<UInt>& edgeEndNodeCache,
                                                           std::vector<UInt>& hangingNodeCache,
                                                           CompoundUndoAction& hangingNodeAction)
{
    for (UInt n = 0; n < numNonHangingNodes; ++n)
    {
        if (hangingNodeCache[n] == constants::missing::uintValue)
        {
            continue;
        }

        auto ee = NextCircularBackwardIndex(n, numNonHangingNodes);
        ee = NextCircularBackwardIndex(ee, numNonHangingNodes);
        const auto eee = NextCircularForwardIndex(n, numNonHangingNodes);

        auto [edgeId1, action1] = m_mesh.ConnectNodes(edgeEndNodeCache[ee], hangingNodeCache[n]);
        hangingNodeAction.Add(std::move(action1));
        auto [edgeId2, action2] = m_mesh.ConnectNodes(edgeEndNodeCache[eee], hangingNodeCache[n]);
        hangingNodeAction.Add(std::move(action2));

        break;
    }
}

void MeshRefinement::ConnectTwoHangingNodesForQuadrilateral(const UInt numNonHangingNodes,
                                                            const std::vector<UInt>& edgeEndNodeCache,
                                                            std::vector<UInt>& hangingNodeCache,
                                                            CompoundUndoAction& hangingNodeAction)
{

    for (UInt n = 0; n < numNonHangingNodes; ++n)
    {
        if (hangingNodeCache[n] == constants::missing::uintValue)
        {
            continue;
        }

        const auto e = NextCircularBackwardIndex(n, numNonHangingNodes);
        const auto ee = NextCircularForwardIndex(n, numNonHangingNodes);
        const auto eee = NextCircularForwardIndex(n + 1, numNonHangingNodes);

        if (hangingNodeCache[e] != constants::missing::uintValue) // left neighbor
        {
            auto [edgeId1, action1] = m_mesh.ConnectNodes(hangingNodeCache[e], hangingNodeCache[n]);
            hangingNodeAction.Add(std::move(action1));
            auto [edgeId2, action2] = m_mesh.ConnectNodes(hangingNodeCache[n], edgeEndNodeCache[ee]);
            hangingNodeAction.Add(std::move(action2));
            auto [edgeId3, action3] = m_mesh.ConnectNodes(edgeEndNodeCache[ee], hangingNodeCache[e]);
            hangingNodeAction.Add(std::move(action3));
        }
        else if (hangingNodeCache[ee] != constants::missing::uintValue) // right neighbor
        {
            auto [edgeId1, action1] = m_mesh.ConnectNodes(hangingNodeCache[n], hangingNodeCache[ee]);
            hangingNodeAction.Add(std::move(action1));
            auto [edgeId2, action2] = m_mesh.ConnectNodes(hangingNodeCache[ee], edgeEndNodeCache[eee]);
            hangingNodeAction.Add(std::move(action2));
            auto [edgeId3, action3] = m_mesh.ConnectNodes(edgeEndNodeCache[eee], hangingNodeCache[n]);
            hangingNodeAction.Add(std::move(action3));
        }
        else if (hangingNodeCache[eee] != constants::missing::uintValue) // hanging nodes must be opposing
        {
            auto [edgeId, action] = m_mesh.ConnectNodes(hangingNodeCache[n], hangingNodeCache[eee]);
            hangingNodeAction.Add(std::move(action));
        }
        break;
    }
}

void MeshRefinement::ConnectOneHangingNodeForTriangle(const UInt numNonHangingNodes,
                                                      const std::vector<UInt>& edgeEndNodeCache,
                                                      std::vector<UInt>& hangingNodeCache,
                                                      CompoundUndoAction& hangingNodeAction)
{

    for (UInt n = 0; n < numNonHangingNodes; ++n)
    {
        if (hangingNodeCache[n] == constants::missing::uintValue)
        {
            continue;
        }
        const auto e = NextCircularForwardIndex(n, numNonHangingNodes);
        auto [edgeId, action] = m_mesh.ConnectNodes(hangingNodeCache[n], edgeEndNodeCache[e]);
        hangingNodeAction.Add(std::move(action));
        break;
    }
}

void MeshRefinement::ConnectTwoHangingNodesForTriangle(const UInt numNonHangingNodes,
                                                       std::vector<UInt>& hangingNodeCache,
                                                       CompoundUndoAction& hangingNodeAction)
{

    for (UInt n = 0; n < numNonHangingNodes; ++n)
    {
        if (hangingNodeCache[n] == constants::missing::uintValue)
        {
            continue;
        }
        const auto e = NextCircularBackwardIndex(n, numNonHangingNodes);
        const auto ee = NextCircularForwardIndex(n, numNonHangingNodes);

        if (hangingNodeCache[e] != constants::missing::uintValue) // left neighbor
        {
            auto [edgeId, action] = m_mesh.ConnectNodes(hangingNodeCache[n], hangingNodeCache[e]);
            hangingNodeAction.Add(std::move(action));
        }
        else
        {
            auto [edgeId, action] = m_mesh.ConnectNodes(hangingNodeCache[n], hangingNodeCache[ee]);
            hangingNodeAction.Add(std::move(action));
        }
        break;
    }
}

std::unique_ptr<meshkernel::UndoAction> MeshRefinement::ConnectHangingNodes()
{
    std::unique_ptr<CompoundUndoAction> hangingNodeAction = CompoundUndoAction::Create();

    std::vector edgeEndNodeCache(constants::geometric::maximumNumberOfNodesPerFace, constants::missing::uintValue);
    std::vector hangingNodeCache(constants::geometric::maximumNumberOfNodesPerFace, constants::missing::uintValue);

    for (UInt f = 0; f < m_mesh.GetNumFaces(); ++f)
    {
        std::ranges::fill(edgeEndNodeCache, constants::missing::uintValue);
        std::ranges::fill(hangingNodeCache, constants::missing::uintValue);
        const auto numEdges = m_mesh.GetNumFaceEdges(f);
        if (numEdges > constants::geometric::maximumNumberOfNodesPerFace)
        {
            continue;
        }

        UInt numNonHangingNodes = 0;
        for (UInt n = 0; n < numEdges; ++n)
        {
            const auto e = NextCircularBackwardIndex(n, numEdges);
            const auto ee = NextCircularForwardIndex(n, numEdges);

            const auto edgeIndex = m_mesh.m_facesEdges[f][n];
            const auto firstEdgeIndex = m_mesh.m_facesEdges[f][e];
            const auto secondEdgeIndex = m_mesh.m_facesEdges[f][ee];
            if (m_brotherEdges[edgeIndex] == secondEdgeIndex)
            {
                continue;
            }

            if (numNonHangingNodes > constants::geometric::maximumNumberOfNodesPerFace - 1)
            {
                return hangingNodeAction;
            }

            edgeEndNodeCache[numNonHangingNodes] = m_mesh.FindCommonNode(edgeIndex, secondEdgeIndex);
            if (edgeEndNodeCache[numNonHangingNodes] == constants::missing::uintValue)
            {
                throw AlgorithmError("Could not find common node.");
            }

            if (m_brotherEdges[edgeIndex] == firstEdgeIndex)
            {
                hangingNodeCache[numNonHangingNodes] = m_mesh.FindCommonNode(edgeIndex, firstEdgeIndex);
                if (hangingNodeCache[numNonHangingNodes] == constants::missing::uintValue)
                {
                    throw AlgorithmError("Could not find common node.");
                }
            }
            numNonHangingNodes++;
        }

        const auto numHangingNodes = numEdges - numNonHangingNodes;

        if (numHangingNodes == 0)
        {
            continue;
        }

        // Quads
        if (numNonHangingNodes == constants::geometric::numNodesInQuadrilateral)
        {
            switch (numHangingNodes)
            {
            case 1: // one hanging node
                ConnectOneHangingNodeForQuadrilateral(numNonHangingNodes, edgeEndNodeCache, hangingNodeCache, *hangingNodeAction);
                break;
            case 2: // two hanging node
                ConnectTwoHangingNodesForQuadrilateral(numNonHangingNodes, edgeEndNodeCache, hangingNodeCache, *hangingNodeAction);
                break;
            default:
                break;
            }
        }
        else if (numNonHangingNodes == constants::geometric::numNodesInTriangle)
        {
            switch (numHangingNodes)
            {
            case 1: // one hanging node
                ConnectOneHangingNodeForTriangle(numNonHangingNodes, edgeEndNodeCache, hangingNodeCache, *hangingNodeAction);
                break;
            case 2: // two hanging node
                ConnectTwoHangingNodesForTriangle(numNonHangingNodes, hangingNodeCache, *hangingNodeAction);
                break;
            default:
                break;
            }
        }
        else
        {
            throw std::invalid_argument("MeshRefinement::connect_hanging_nodes: The number of non-hanging nodes is neither 3 nor 4.");
        }
    }

    return hangingNodeAction;
}

meshkernel::Point MeshRefinement::ComputeMidPoint(const Point& firstNode, const Point& secondNode) const
{
    meshkernel::Point midPoint = 0.5 * (firstNode + secondNode);

    if (m_mesh.m_projection == Projection::spherical)
    {

        midPoint.y = (firstNode.y + secondNode.y) / 2.0;

        if (std::abs(firstNode.x - secondNode.x) > 180.0)
        {
            midPoint.x += 180.0;
        }

        // fix at poles
        const auto firstNodeAtPole = IsPointOnPole(firstNode);
        const auto secondNodeAtPole = IsPointOnPole(secondNode);
        if (firstNodeAtPole && !secondNodeAtPole)
        {
            midPoint.x = secondNode.x;
        }
        else if (!firstNodeAtPole && secondNodeAtPole)
        {
            midPoint.x = firstNode.x;
        }
    }

    return midPoint;
}

int MeshRefinement::DetermineNodeMaskValue(const int firstNodeMask, const int secondNodeMask) const
{
    int maskValue = 1;

    if (firstNodeMask == 0 && secondNodeMask == 0)
    {
        maskValue = 0;
    }
    else if (firstNodeMask != 1 || secondNodeMask != 1)
    {
        maskValue = -1;
    }

    return maskValue;
}

bool MeshRefinement::DetermineIfParentIsCrossed(const UInt faceId, const UInt numEdges) const
{
    bool isParentCrossed = false;
    for (UInt e = 0; e < numEdges; ++e)
    {
        const auto n = m_mesh.m_facesNodes[faceId][e];
        if (m_nodeMask[n] != 1)
        {
            isParentCrossed = true;
            break;
        }
    }

    return isParentCrossed;
}

bool MeshRefinement::FindNonHangingNodeEdges(const UInt faceId,
                                             const UInt numEdges,
                                             std::vector<UInt>& notHangingFaceNodes,
                                             std::vector<UInt>& nonHangingEdges,
                                             UInt& numBrotherEdges) const
{
    for (UInt e = 0; e < numEdges; e++)
    {
        const auto firstEdge = NextCircularBackwardIndex(e, numEdges);
        const auto secondEdge = NextCircularForwardIndex(e, numEdges);

        auto mappedEdge = m_localNodeIndicesCache[e];
        const auto edgeIndex = m_mesh.m_facesEdges[faceId][mappedEdge];

        mappedEdge = m_localNodeIndicesCache[firstEdge];
        const auto firstEdgeIndex = m_mesh.m_facesEdges[faceId][mappedEdge];

        mappedEdge = m_localNodeIndicesCache[secondEdge];
        const auto secondEdgeIndex = m_mesh.m_facesEdges[faceId][mappedEdge];

        if (edgeIndex == constants::missing::uintValue)
        {
            continue;
        }

        if (m_brotherEdges[edgeIndex] == secondEdgeIndex && secondEdgeIndex != constants::missing::uintValue)
        {
            numBrotherEdges++;
            const auto newNode = m_mesh.FindCommonNode(edgeIndex, m_brotherEdges[edgeIndex]);

            if (newNode == constants::missing::uintValue)
            {
                throw AlgorithmError("Could not find common node.");
            }

            notHangingFaceNodes.emplace_back(newNode);
        }
        else if ((m_brotherEdges[edgeIndex] != firstEdgeIndex || m_brotherEdges[edgeIndex] == constants::missing::uintValue) && m_edgeMask[edgeIndex] != 0)
        {
            notHangingFaceNodes.emplace_back(m_edgeMask[edgeIndex]);
        }

        if (notHangingFaceNodes.size() >= constants::geometric::maximumNumberOfNodesPerFace)
        {
            return true;
        }

        // check if start of this link is hanging
        if (m_brotherEdges[edgeIndex] != firstEdgeIndex || firstEdgeIndex != constants::missing::uintValue)
        {
            nonHangingEdges.emplace_back(e);
        }
    }

    return false;
}

void MeshRefinement::FindFacePolygonWithoutHangingNodes(const UInt faceId,
                                                        const std::vector<UInt>& nonHangingEdges,
                                                        std::vector<Point>& facePolygonWithoutHangingNodes,
                                                        std::vector<UInt>& localEdgesNumFaces) const
{
    for (const auto& edge : nonHangingEdges)
    {
        facePolygonWithoutHangingNodes.emplace_back(m_polygonNodesCache[edge]);

        const auto mappedEdge = m_localNodeIndicesCache[edge];
        const auto edgeIndex = m_mesh.m_facesEdges[faceId][mappedEdge];

        if (edgeIndex != constants::missing::uintValue)
        {
            localEdgesNumFaces.emplace_back(m_mesh.GetNumEdgesFaces(edgeIndex));
        }
        else
        {
            localEdgesNumFaces.emplace_back(1);
        }
    }
}

void MeshRefinement::ComputeSplittingNode(const UInt faceId,
                                          std::vector<Point>& facePolygonWithoutHangingNodes,
                                          std::vector<UInt>& localEdgesNumFaces,
                                          Point& splittingNode) const
{
    splittingNode = m_mesh.m_facesMassCenters[faceId];

    if (localEdgesNumFaces.size() == constants::geometric::numNodesInQuadrilateral && m_meshRefinementParameters.use_mass_center_when_refining == 0)
    {
        // close the polygon before computing the face circumcenter
        facePolygonWithoutHangingNodes.emplace_back(facePolygonWithoutHangingNodes.front());
        localEdgesNumFaces.emplace_back(localEdgesNumFaces.front());

        splittingNode = ComputeFaceCircumenter(facePolygonWithoutHangingNodes,
                                               localEdgesNumFaces,
                                               m_mesh.m_projection);

        if (m_mesh.m_projection == Projection::spherical)
        {
            auto miny = std::numeric_limits<double>::max();
            auto maxy = std::numeric_limits<double>::lowest();
            for (const auto& node : facePolygonWithoutHangingNodes)
            {
                miny = std::min(node.y, miny);
                maxy = std::max(node.y, maxy);
            }

            const auto middlelatitude = (miny + maxy) / 2.0;
            const auto ydiff = maxy - miny;
            if (ydiff > 1e-8)
            {
                splittingNode.y = miny + 2.0 * (middlelatitude - miny) / ydiff * (splittingNode.y - miny);
            }
        }
    }
}

void MeshRefinement::SplitEdges(const bool isParentCrossed,
                                const std::vector<UInt>& localEdgesNumFaces,
                                const std::vector<UInt>& notHangingFaceNodes,
                                const Point& splittingNode,
                                CompoundUndoAction& refineFacesAction)
{

    if (localEdgesNumFaces.size() >= constants::geometric::numNodesInQuadrilateral)
    {
        if (notHangingFaceNodes.size() > 2)
        {
            auto [newNodeIndex, insertAction] = m_mesh.InsertNode(splittingNode);
            refineFacesAction.Add(std::move(insertAction));

            for (const auto& notHangingNode : notHangingFaceNodes)
            {
                auto [edgeId, action] = m_mesh.ConnectNodes(notHangingNode, newNodeIndex);
                refineFacesAction.Add(std::move(action));
            }

            m_nodeMask.emplace_back(1);
            if (isParentCrossed)
            {
                // inactive nodes in cells crossed by polygon
                m_nodeMask[newNodeIndex] = -1;
            }
        }
        else if (notHangingFaceNodes.size() == 2)
        {
            auto [edgeId, action] = m_mesh.ConnectNodes(notHangingFaceNodes[0], notHangingFaceNodes[1]);
            refineFacesAction.Add(std::move(action));
        }
    }
    else
    {
        for (UInt n = 0; n < notHangingFaceNodes.size(); ++n)
        {
            const auto nn = NextCircularForwardIndex(n, static_cast<UInt>(notHangingFaceNodes.size()));
            auto [edgeId, action] = m_mesh.ConnectNodes(notHangingFaceNodes[n], notHangingFaceNodes[nn]);
            refineFacesAction.Add(std::move(action));
        }
    }
}

std::unique_ptr<meshkernel::UndoAction> MeshRefinement::RefineFacesBySplittingEdges()
{
    const auto numEdgesBeforeRefinement = m_mesh.GetNumEdges();

    std::unique_ptr<CompoundUndoAction> refineFacesAction = CompoundUndoAction::Create();

    // Add new nodes where required
    std::vector<UInt> notHangingFaceNodes;
    notHangingFaceNodes.reserve(constants::geometric::maximumNumberOfNodesPerFace);
    std::vector<UInt> nonHangingEdges;
    nonHangingEdges.reserve(constants::geometric::maximumNumberOfNodesPerFace);

    std::vector<Point> facePolygonWithoutHangingNodes;
    facePolygonWithoutHangingNodes.reserve(constants::geometric::maximumNumberOfNodesPerFace);
    std::vector<UInt> localEdgesNumFaces;
    localEdgesNumFaces.reserve(constants::geometric::maximumNumberOfEdgesPerFace);

    for (UInt e = 0; e < m_mesh.GetNumEdges(); e++)
    {
        if (m_edgeMask[e] == 0)
        {
            continue;
        }

        // Compute the center of the edge
        const auto firstNodeIndex = m_mesh.GetEdge(e).first;
        const auto secondNodeIndex = m_mesh.GetEdge(e).second;
        const auto firstNode = m_mesh.Node(firstNodeIndex);
        const auto secondNode = m_mesh.Node(secondNodeIndex);

        Point middle = ComputeMidPoint(firstNode, secondNode);

        auto [newNodeIndex, insertAction] = m_mesh.InsertNode(middle);
        refineFacesAction->Add(std::move(insertAction));
        m_edgeMask[e] = static_cast<int>(newNodeIndex);

        // set mask on the new node
        m_nodeMask.emplace_back(DetermineNodeMaskValue(m_nodeMask[firstNodeIndex], m_nodeMask[secondNodeIndex]));
    }

    for (UInt f = 0; f < m_mesh.GetNumFaces(); f++)
    {
        if (m_faceMask[f] == 0)
        {
            continue;
        }

        const auto numEdges = m_mesh.GetNumFaceEdges(f);

        m_mesh.ComputeFaceClosedPolygonWithLocalMappings(f, m_polygonNodesCache, m_localNodeIndicesCache, m_globalEdgeIndicesCache);

        UInt numBrotherEdges = 0;
        notHangingFaceNodes.clear();
        nonHangingEdges.clear();

        if (FindNonHangingNodeEdges(f, numEdges, notHangingFaceNodes, nonHangingEdges, numBrotherEdges))
        {
            return refineFacesAction;
        }

        // compute new center node : circumcenter without hanging nodes for quads, c / g otherwise
        facePolygonWithoutHangingNodes.clear();
        localEdgesNumFaces.clear();

        FindFacePolygonWithoutHangingNodes(f, nonHangingEdges, facePolygonWithoutHangingNodes, localEdgesNumFaces);

        Point splittingNode;
        ComputeSplittingNode(f, facePolygonWithoutHangingNodes, localEdgesNumFaces, splittingNode);

        // check if the parent face is crossed by the enclosing polygon
        bool isParentCrossed = DetermineIfParentIsCrossed(f, numEdges);

        SplitEdges(isParentCrossed, localEdgesNumFaces, notHangingFaceNodes, splittingNode, *refineFacesAction);
    }

    // Split original edges
    for (UInt e = 0; e < numEdgesBeforeRefinement; ++e)
    {
        if (m_edgeMask[e] > 0)
        {
            auto [newEdgeIndex, connectAction] = m_mesh.ConnectNodes(m_edgeMask[e], m_mesh.GetEdge(e).second);
            refineFacesAction->Add(std::move(connectAction));

            refineFacesAction->Add(m_mesh.ResetEdge(e, {m_mesh.GetEdge(e).first, m_edgeMask[e]}));
            m_brotherEdges.resize(m_mesh.GetNumEdges());
            m_brotherEdges[newEdgeIndex] = e;
            m_brotherEdges[e] = newEdgeIndex;
        }
    }

    return refineFacesAction;
}

void MeshRefinement::ComputeNodeMaskAtPolygonPerimeter()
{
    for (UInt f = 0; f < m_mesh.GetNumFaces(); f++)
    {
        bool crossing = false;
        const auto numnodes = m_mesh.GetNumFaceEdges(f);
        for (UInt n = 0; n < numnodes; n++)
        {
            const auto nodeIndex = m_mesh.m_facesNodes[f][n];
            if (m_nodeMask[nodeIndex] == 0)
            {
                crossing = true;
                break;
            }
        }

        if (crossing)
        {
            m_faceMask[f] = 0;
            for (UInt n = 0; n < numnodes; n++)
            {
                const auto nodeIndex = m_mesh.m_facesNodes[f][n];
                if (m_nodeMask[nodeIndex] == 1)
                {
                    m_nodeMask[nodeIndex] = -2;
                }
            }
        }
    }
}

void MeshRefinement::ComputeRefinementMasksFromSamples()
{
    std::ranges::fill(m_edgeMask, 0);
    std::ranges::fill(m_faceMask, 0);

    m_polygonNodesCache.resize(constants::geometric::maximumNumberOfNodesPerFace + 1);
    m_localNodeIndicesCache.resize(constants::geometric::maximumNumberOfNodesPerFace + 1, constants::missing::uintValue);
    m_globalEdgeIndicesCache.resize(constants::geometric::maximumNumberOfEdgesPerFace + 1, constants::missing::uintValue);
    m_refineEdgeCache.resize(constants::geometric::maximumNumberOfEdgesPerFace, 0);

    // Compute all interpolated values
    m_interpolant->Compute();

    if (m_useNodalRefinement && m_refinementType == RefinementType::WaveCourant)
    {
        ComputeFaceLocationTypes();
    }

    for (UInt f = 0; f < m_mesh.GetNumFaces(); f++)
    {
        FindHangingNodes(f);

        ComputeRefinementMasksFromSamples(f);
    }

    for (auto& edge : m_edgeMask)
    {
        edge = -edge;
    }
    SmoothRefinementMasks();
}

void MeshRefinement::FindHangingNodes(UInt face)
{
    const auto numFaceNodes = m_mesh.GetNumFaceEdges(face);

    if (numFaceNodes > constants::geometric::maximumNumberOfEdgesPerNode)
    {
        throw AlgorithmError("The number of face nodes is greater than the maximum number of edges per node.");
    }

    m_isHangingNodeCache.resize(constants::geometric::maximumNumberOfNodesPerFace);
    m_isHangingEdgeCache.resize(constants::geometric::maximumNumberOfEdgesPerFace);
    std::fill(m_isHangingNodeCache.begin(), m_isHangingNodeCache.end(), false);
    std::fill(m_isHangingEdgeCache.begin(), m_isHangingEdgeCache.end(), false);

    auto kknod = numFaceNodes - 1;

    for (UInt n = 0; n < numFaceNodes; n++)
    {
        const auto edgeIndex = m_mesh.m_facesEdges[face][n];

        // check if the parent edge is in the cell
        if (m_brotherEdges[edgeIndex] != constants::missing::uintValue)
        {
            const auto e = NextCircularBackwardIndex(n, numFaceNodes);
            const auto ee = NextCircularForwardIndex(n, numFaceNodes);
            const auto firstEdgeIndex = m_mesh.m_facesEdges[face][e];
            const auto secondEdgeIndex = m_mesh.m_facesEdges[face][ee];

            UInt commonNode = constants::missing::uintValue;
            if (m_brotherEdges[edgeIndex] == firstEdgeIndex)
            {
                commonNode = m_mesh.FindCommonNode(edgeIndex, firstEdgeIndex);
                if (commonNode == constants::missing::uintValue)
                {
                    throw AlgorithmError("Could not find common node.");
                }
            }
            else if (m_brotherEdges[edgeIndex] == secondEdgeIndex)
            {
                commonNode = m_mesh.FindCommonNode(edgeIndex, secondEdgeIndex);
                if (commonNode == constants::missing::uintValue)
                {
                    throw AlgorithmError("Could not find common node.");
                }
            }

            if (commonNode != constants::missing::uintValue)
            {
                m_isHangingEdgeCache[n] = true;
                for (UInt nn = 0; nn < numFaceNodes; nn++)
                {
                    kknod = NextCircularForwardIndex(kknod, numFaceNodes);

                    if (m_mesh.m_facesNodes[face][kknod] == commonNode && !m_isHangingNodeCache[kknod])
                    {
                        m_isHangingNodeCache[kknod] = true;
                        break;
                    }
                }
            }
        }
    }
}

meshkernel::UInt MeshRefinement::CountHangingNodes() const
{
    meshkernel::UInt result = 0;
    for (const auto& v : m_isHangingNodeCache)
    {
        if (v)
        {
            result++;
        }
    }
    return result;
}

meshkernel::UInt MeshRefinement::CountHangingEdges() const
{
    meshkernel::UInt result = 0;
    for (const auto& v : m_isHangingEdgeCache)
    {
        if (v)
        {
            result++;
        }
    }
    return result;
}

meshkernel::UInt MeshRefinement::CountEdgesToRefine(UInt face) const
{
    const auto numFaceNodes = m_mesh.GetNumFaceEdges(face);

    UInt result = 0;

    for (UInt n = 0; n < numFaceNodes; n++)
    {
        const auto edgeIndex = m_mesh.m_facesEdges[face][n];
        if (m_edgeMask[edgeIndex] != 0)
        {
            result += 1;
        }
    }
    return result;
}

void MeshRefinement::ComputeRefinementMasksForRefinementLevels(UInt face,
                                                               size_t& numberOfEdgesToRefine,
                                                               std::vector<UInt>& edgeToRefine) const
{
    if (m_interpolant->GetFaceResult(face) <= 0)
    {
        return;
    }

    for (UInt i = 0; i < m_mesh.GetNumFaceEdges(face); i++)
    {
        numberOfEdgesToRefine++;
        edgeToRefine[i] = 1;
    }
}

bool MeshRefinement::DetermineRequiredRefinement(const UInt face,
                                                 const UInt edge) const
{
    bool doRefinement = false;

    if (m_useNodalRefinement)
    {
        if (m_faceLocationType[face] == FaceLocation::Land)
        {
            doRefinement = false;
        }
        else if (m_faceLocationType[face] == FaceLocation::LandWater)
        {
            doRefinement = true;
        }
        else
        {
            const auto edgeDepth = std::abs(m_interpolant->GetEdgeResult(edge));
            doRefinement = IsRefineNeededBasedOnCourantCriteria(edge, edgeDepth);
        }
    }
    else
    {
        const double faceDepth = m_interpolant->GetFaceResult(face);
        doRefinement = IsRefineNeededBasedOnCourantCriteria(edge, faceDepth);
    }

    return doRefinement;
}

void MeshRefinement::ResetNumberOfEdgesToRefineForFace(const UInt face,
                                                       const std::vector<UInt>& edgeToRefine,
                                                       size_t& numberOfEdgesToRefine) const
{
    numberOfEdgesToRefine = 0;

    for (UInt i = 0; i < m_mesh.GetNumFaceEdges(face); i++)
    {
        if (edgeToRefine[i] == 1 || m_isHangingNodeCache[i])
        {
            numberOfEdgesToRefine++;
        }
    }
}

void MeshRefinement::DetermineEdgesToRefine(const UInt face,
                                            std::vector<UInt>& edgeToRefine,
                                            size_t& numberOfEdgesToRefine) const
{
    if (numberOfEdgesToRefine == m_mesh.GetNumFaceEdges(face))
    {
        for (UInt i = 0; i < m_mesh.GetNumFaceEdges(face); i++)
        {
            if (!m_isHangingNodeCache[i])
            {
                edgeToRefine[i] = 1;
            }
        }
    }
    else
    {
        numberOfEdgesToRefine = 0;
    }
}

void MeshRefinement::ComputeRefinementMasksForWaveCourant(UInt face,
                                                          size_t& numberOfEdgesToRefine,
                                                          std::vector<UInt>& edgeToRefine)
{
    for (size_t e = 0; e < m_mesh.GetNumFaceEdges(face); ++e)
    {
        const auto edge = m_mesh.m_facesEdges[face][e];
        if (m_edgeLengths[edge] < m_mergingDistance)
        {
            numberOfEdgesToRefine++;
            continue;
        }

        if (m_isEdgeBelowMinSizeAfterRefinement[edge])
        {
            continue;
        }

        bool doRefinement = DetermineRequiredRefinement(face, edge);

        if (doRefinement)
        {
            numberOfEdgesToRefine++;
            edgeToRefine[e] = 1;
        }
    }

    if (numberOfEdgesToRefine > 0)
    {
        ResetNumberOfEdgesToRefineForFace(face, edgeToRefine, numberOfEdgesToRefine);
    }

    if (m_meshRefinementParameters.directional_refinement == 0)
    {
        DetermineEdgesToRefine(face, edgeToRefine, numberOfEdgesToRefine);
    }
}

void MeshRefinement::ComputeRefinementMasksForRidgeDetection(UInt face,
                                                             size_t& numberOfEdgesToRefine,
                                                             std::vector<UInt>& edgeToRefine) const
{
    double maxEdgeLength = 0.0;
    UInt numEdges = m_mesh.GetNumFaceEdges(face);

    for (size_t i = 0; i < numEdges; ++i)
    {

        const auto& edgeIndex = m_mesh.m_facesEdges[face][i];
        const auto& [firstNode, secondNode] = m_mesh.GetEdge(edgeIndex);
        const auto distance = ComputeDistance(m_mesh.Node(firstNode), m_mesh.Node(secondNode), m_mesh.m_projection);
        maxEdgeLength = std::max(maxEdgeLength, distance);
    }

    double absInterpolatedFaceValue = std::abs(m_interpolant->GetFaceResult(face));
    double threshold = 1.0; // In Fortran code this value is 100
    double thresholdMin = 1.0;
    double hmin = m_meshRefinementParameters.min_edge_size;

    double elementSizeWanted = threshold / (absInterpolatedFaceValue + 1.0e-8);

    if (maxEdgeLength > elementSizeWanted && maxEdgeLength > 2.0 * hmin && absInterpolatedFaceValue > thresholdMin)
    {
        numberOfEdgesToRefine += numEdges;

        for (UInt i = 0; i < numEdges; i++)
        {
            edgeToRefine[i] = 1;
        }
    }
}

void MeshRefinement::ComputeRefinementMasksFromSamples(UInt face)
{
    if (IsEqual(m_interpolant->GetFaceResult(face), constants::missing::doubleValue))
    {
        return;
    }

    size_t numEdgesToBeRefined = 0;
    std::ranges::fill(m_refineEdgeCache, 0);

    switch (m_refinementType)
    {
    case RefinementType::RefinementLevels:
        ComputeRefinementMasksForRefinementLevels(face, numEdgesToBeRefined, m_refineEdgeCache);
        break;

    case RefinementType::WaveCourant:
        ComputeRefinementMasksForWaveCourant(face, numEdgesToBeRefined, m_refineEdgeCache);
        break;

    case RefinementType::RidgeDetection:
        ComputeRefinementMasksForRidgeDetection(face, numEdgesToBeRefined, m_refineEdgeCache);
        break;

    default:
        throw AlgorithmError("Invalid refinement type");
    }

    // Compute face and edge masks
    if (numEdgesToBeRefined > 1)
    {
        m_faceMask[face] = 1;

        for (size_t n = 0; n < m_mesh.GetNumFaceEdges(face); n++)
        {
            if (m_refineEdgeCache[n] == 1)
            {
                const auto edgeIndex = m_mesh.m_facesEdges[face][n];
                if (edgeIndex != constants::missing::uintValue)
                {
                    m_edgeMask[edgeIndex] = 1;
                }
            }
        }
    }
}

void MeshRefinement::ComputeFaceLocationTypes()
{
    m_faceLocationType.resize(m_mesh.GetNumFaces());
    std::ranges::fill(m_faceLocationType, FaceLocation::Water);
    for (UInt face = 0; face < m_mesh.GetNumFaces(); face++)
    {
        double maxVal = -std::numeric_limits<double>::max();
        double minVal = std::numeric_limits<double>::max();
        for (UInt e = 0; e < m_mesh.GetNumFaceEdges(face); ++e)
        {
            const auto node = m_mesh.m_facesNodes[face][e];
            const auto val = m_interpolant->GetNodeResult(node);
            maxVal = std::max(maxVal, val);
            minVal = std::min(minVal, val);
        }

        if (minVal > 0.0)
        {
            m_faceLocationType[face] = FaceLocation::Land;
        }
        if (maxVal >= 0.0 && (!IsEqual(minVal, constants::missing::doubleValue) && minVal < 0.0))
        {
            m_faceLocationType[face] = FaceLocation::LandWater;
        }
    }
}

void MeshRefinement::ComputeEdgeBelowMinSizeAfterRefinement()
{
    m_edgeLengths = algo::ComputeMeshEdgeLength(m_mesh);

    m_isEdgeBelowMinSizeAfterRefinement.resize(m_mesh.GetNumEdges());
    for (UInt e = 0; e < m_mesh.GetNumEdges(); e++)
    {
        if (IsEqual(m_edgeLengths[e], constants::missing::doubleValue))
        {
            m_isEdgeBelowMinSizeAfterRefinement[e] = true; // The invalid edge will not be refined
            continue;
        }
        const double newEdgeLength = 0.5 * m_edgeLengths[e];
        m_isEdgeBelowMinSizeAfterRefinement[e] = newEdgeLength < m_meshRefinementParameters.min_edge_size;
    }
}

bool MeshRefinement::IsRefineNeededBasedOnCourantCriteria(UInt edge, double depthValues) const
{
    const double maxDtCourant = m_meshRefinementParameters.max_courant_time;
    const double celerity = constants::physical::sqrt_gravity * std::sqrt(std::abs(depthValues));
    const double waveCourant = celerity * maxDtCourant / m_edgeLengths[edge];
    return waveCourant < 1.0;
}

void MeshRefinement::ComputeEdgesRefinementMask()
{
    bool repeat = true;
    UInt iter = 0;
    const UInt numMaxIterations = 6;
    std::vector<int> isQuadEdge(constants::geometric::numNodesInQuadrilateral);
    std::vector<UInt> numOfEdges(constants::geometric::maximumNumberOfEdgesPerFace);

    while (repeat && iter < numMaxIterations)
    {
        repeat = false;
        iter++;

        for (UInt f = 0; f < m_mesh.GetNumFaces(); f++)
        {
            if (m_faceMask[f] == 0)
            {
                continue;
            }

            const auto numHangingNodes = CountHangingEdges();

            const auto numFaceNodes = m_mesh.GetNumFaceEdges(f);

            // non-quads
            const auto numNodesEffective = numFaceNodes - numHangingNodes;
            if (numNodesEffective != constants::geometric::numNodesInQuadrilateral)
            {
                for (UInt n = 0; n < numFaceNodes; n++)
                {
                    const auto e = NextCircularBackwardIndex(n, numFaceNodes);
                    const auto ee = NextCircularForwardIndex(n, numFaceNodes);

                    const auto edgeIndex = m_mesh.m_facesEdges[f][n];

                    const auto firstEdgeIndex = m_mesh.m_facesEdges[f][e];
                    const auto secondEdgeIndex = m_mesh.m_facesEdges[f][ee];

                    // do not refine edges with an hanging node
                    if (m_brotherEdges[edgeIndex] != firstEdgeIndex && m_brotherEdges[edgeIndex] != secondEdgeIndex)
                    {
                        m_edgeMask[edgeIndex] = 1;
                    }
                }
            }
            if (numNodesEffective == constants::geometric::numNodesInQuadrilateral)
            {
                // number the links in the cell, links that share a hanging node will have the same number
                UInt num = 0;
                for (UInt n = 0; n < numFaceNodes; n++)
                {
                    const auto edgeIndex = m_mesh.m_facesEdges[f][n];
                    numOfEdges[n] = num;

                    if (m_edgeMask[edgeIndex] != 0)
                    {
                        isQuadEdge[num] = m_edgeMask[edgeIndex];
                    }

                    const auto ee = NextCircularForwardIndex(n, numFaceNodes);

                    const auto secondEdgeIndex = m_mesh.m_facesEdges[f][ee];

                    if (n != numFaceNodes - 1 && m_brotherEdges[edgeIndex] != secondEdgeIndex)
                    {
                        num++;
                    }
                    if (m_brotherEdges[edgeIndex] == secondEdgeIndex)
                    {
                        isQuadEdge[num] = 1;
                    }
                }

                if (num + 1 != constants::geometric::numNodesInQuadrilateral)
                {
                    throw AlgorithmError("The number the links in the cell is not equal to 3.");
                }

                UInt numEdgesToRefine = 0;
                UInt firstEdgeIndex = 0;
                UInt secondEdgeIndex = 0;
                for (UInt i = 0; i < constants::geometric::numNodesInQuadrilateral; i++)
                {
                    if (isQuadEdge[i] != 0)
                    {
                        numEdgesToRefine++;
                        if (firstEdgeIndex == 0)
                        {
                            firstEdgeIndex = i;
                        }
                        else if (secondEdgeIndex == 0)
                        {
                            secondEdgeIndex = i;
                        }
                    }
                }

                const auto edgeIndexDifference = secondEdgeIndex - firstEdgeIndex;
                bool refineAllEdges = false;
                if (numEdgesToRefine == 2 && (edgeIndexDifference == 1 || edgeIndexDifference == 3))
                {
                    repeat = true;
                    refineAllEdges = true;
                }

                for (UInt n = 0; n < numFaceNodes; n++)
                {
                    const auto edgeIndex = m_mesh.m_facesEdges[f][n];
                    if (m_edgeMask[edgeIndex] > 0)
                    {
                        continue;
                    }

                    if (refineAllEdges != true && m_edgeMask[edgeIndex] != -1)
                    {
                        continue;
                    }

                    const auto e = NextCircularBackwardIndex(n, numFaceNodes);
                    const auto ee = NextCircularForwardIndex(n, numFaceNodes);

                    if (numOfEdges[n] != numOfEdges[e] && numOfEdges[n] != numOfEdges[ee])
                    {
                        m_edgeMask[edgeIndex] = 1;
                    }
                }
            }
        }
    }

    if (repeat)
    {
        throw AlgorithmError("Solution did not converge.");
    }

    // only keep m_edgeMask = 1, set other values to 0
    for (auto& value : m_edgeMask)
    {
        if (value != 1)
        {
            value = 0;
        }
    }
}

bool MeshRefinement::IsSplittingIsRequiredForFace(const UInt faceId) const
{
    const auto numFaceNodes = m_mesh.GetNumFaceEdges(faceId);

    const auto numHangingEdges = CountHangingEdges();
    const auto numHangingNodes = CountHangingNodes();
    const auto numEdgesToRefine = CountEdgesToRefine(faceId);

    bool isSplittingRequired = false;

    for (UInt n = 0; n < numFaceNodes; n++)
    {
        const auto edgeIndex = m_mesh.m_facesEdges[faceId][n];

        if (m_isHangingEdgeCache[n] && m_edgeMask[edgeIndex] > 0)
        {
            isSplittingRequired = true;
            break;
        }
    }

    // compute the effective face type
    const auto numNodesEffective = numFaceNodes - static_cast<UInt>(static_cast<double>(numHangingEdges) / 2.0);

    if (numFaceNodes + numEdgesToRefine > constants::geometric::maximumNumberOfEdgesPerFace || // would result in unsupported cells after refinement
        numFaceNodes - numHangingNodes - numEdgesToRefine <= 1 ||                              // faces with only one unrefined edge
        numNodesEffective == numEdgesToRefine)                                                 // refine all edges
    {
        isSplittingRequired = true;
    }

    return isSplittingRequired;
}

meshkernel::UInt MeshRefinement::UpdateEdgeMaskForNonHangingEdge(const UInt faceId,
                                                                 const UInt numFaceNodes,
                                                                 const UInt iter,
                                                                 const UInt maxiter)
{
    UInt num = 0;

    for (UInt n = 0; n < numFaceNodes; n++)
    {
        const auto edgeIndex = m_mesh.m_facesEdges[faceId][n];

        if (!m_isHangingEdgeCache[n] && m_edgeMask[edgeIndex] == 0)
        {
            m_edgeMask[edgeIndex] = 1;
            num++;
        }

        if (iter == maxiter)
        {
            throw AlgorithmError("Problem with vertex and edge");
        }
    }

    return num;
}

void MeshRefinement::ComputeIfFaceShouldBeSplit()
{
    const UInt maxiter = 1000;
    UInt num = 1;
    UInt iter = 0;
    while (num != 0)
    {
        iter++;
        if (iter > maxiter)
        {
            break;
        }

        num = 0;
        for (UInt f = 0; f < m_mesh.GetNumFaces(); f++)
        {
            if (m_faceMask[f] != 0 && m_faceMask[f] != -1)
            {
                continue;
            }

            FindHangingNodes(f);

            // check if the edge has a brother edge and needs to be refined
            const auto numFaceNodes = m_mesh.GetNumFaceEdges(f);

            if (numFaceNodes > constants::geometric::maximumNumberOfEdgesPerFace)
            {
                return;
            }

            bool isSplittingRequired = IsSplittingIsRequiredForFace(f);

            // compute the effective face type
            const auto numHangingEdges = CountHangingEdges();
            const auto numNodesEffective = numFaceNodes - static_cast<UInt>(static_cast<double>(numHangingEdges) / 2.0);

            if (2 * (numFaceNodes - numNodesEffective) != numHangingEdges)
            {
                // uneven number of brotherlinks
                // TODO: ADD DOT
            }

            if (isSplittingRequired)
            {

                if (m_faceMask[f] != -1)
                {
                    m_faceMask[f] = 2;
                }
                else
                {
                    m_faceMask[f] = -2;
                }

                num += UpdateEdgeMaskForNonHangingEdge(f, numFaceNodes, iter, maxiter);
            }
        }
    }
}

void MeshRefinement::FindBrotherEdges()
{
    m_brotherEdges.resize(m_mesh.GetNumEdges());
    std::ranges::fill(m_brotherEdges, constants::missing::uintValue);

    for (UInt n = 0; n < m_mesh.GetNumNodes(); n++)
    {
        const auto numEdgesNodes = m_mesh.GetNumNodesEdges(n);
        for (UInt e = 0; e < numEdgesNodes; e++)
        {

            const auto firstEdgeIndex = m_mesh.m_nodesEdges[n][e];
            if (m_mesh.GetNumEdgesFaces(firstEdgeIndex) < 1)
            {
                continue;
            }

            const auto ee = NextCircularForwardIndex(e, numEdgesNodes);
            const auto secondEdgeIndex = m_mesh.m_nodesEdges[n][ee];
            if (m_mesh.GetNumEdgesFaces(secondEdgeIndex) < 1)
            {
                continue;
            }

            // both edges should share the same face
            const auto firstEdgeLeftFace = m_mesh.m_edgesFaces[firstEdgeIndex][0];
            const auto firstEdgeRighFace = m_mesh.GetNumEdgesFaces(firstEdgeIndex) == 1 ? firstEdgeLeftFace : m_mesh.m_edgesFaces[firstEdgeIndex][1];
            const auto secondEdgeLeftFace = m_mesh.m_edgesFaces[secondEdgeIndex][0];
            const auto secondEdgeRighFace = m_mesh.GetNumEdgesFaces(secondEdgeIndex) == 1 ? secondEdgeLeftFace : m_mesh.m_edgesFaces[secondEdgeIndex][1];

            if (firstEdgeLeftFace != secondEdgeLeftFace &&
                firstEdgeLeftFace != secondEdgeRighFace &&
                firstEdgeRighFace != secondEdgeLeftFace &&
                firstEdgeRighFace != secondEdgeRighFace)
            {
                continue;
            }

            // check if node k is in the middle
            const auto firstEdgeOtherNode = OtherNodeOfEdge(m_mesh.GetEdge(firstEdgeIndex), n);
            const auto secondEdgeOtherNode = OtherNodeOfEdge(m_mesh.GetEdge(secondEdgeIndex), n);
            const auto center = ComputeMiddlePointAccountingForPoles(m_mesh.Node(firstEdgeOtherNode), m_mesh.Node(secondEdgeOtherNode), m_mesh.m_projection);

            // compute tolerance
            const auto firstEdgeLength = ComputeDistance(m_mesh.Node(firstEdgeOtherNode), m_mesh.Node(n), m_mesh.m_projection);
            const auto secondEdgeLength = ComputeDistance(m_mesh.Node(secondEdgeOtherNode), m_mesh.Node(n), m_mesh.m_projection);
            const auto minConnectionDistance = 1e-4 * std::max(firstEdgeLength, secondEdgeLength);

            // The center of the two edges coincides with the shared node
            const auto distanceFromCentre = ComputeDistance(center, m_mesh.Node(n), m_mesh.m_projection);
            if (distanceFromCentre < minConnectionDistance)
            {
                m_brotherEdges[firstEdgeIndex] = secondEdgeIndex;
                m_brotherEdges[secondEdgeIndex] = firstEdgeIndex;
            }
        }
    }
}

void MeshRefinement::FindEdgesToSplit(const UInt faceId,
                                      const UInt numEdges,
                                      std::vector<bool>& splitEdge) const
{
    for (UInt e = 0; e < numEdges; ++e)
    {
        const auto edgeIndex = m_mesh.m_facesEdges[faceId][e];
        const auto nextEdgeIndex = NextCircularForwardIndex(e, numEdges);
        const auto previousEdgeIndex = NextCircularBackwardIndex(e, numEdges);
        const auto split = m_brotherEdges[edgeIndex] != m_mesh.m_facesEdges[faceId][nextEdgeIndex] &&
                           m_brotherEdges[edgeIndex] != m_mesh.m_facesEdges[faceId][previousEdgeIndex];

        if (split)
        {
            splitEdge[edgeIndex] = true;
        }
    }
}

void MeshRefinement::UpdateFaceRefinementMask(std::vector<bool>& splitEdge)
{
    for (UInt f = 0; f < m_mesh.GetNumFaces(); ++f)
    {
        const auto numEdges = m_mesh.GetNumFaceEdges(f);

        for (UInt e = 0; e < numEdges; ++e)
        {
            const auto edgeIndex = m_mesh.m_facesEdges[f][e];
            if (splitEdge[edgeIndex])
            {
                m_faceMask[f] = 1;
            }
        }
    }
}

void MeshRefinement::UpdateEdgeRefinementMask()
{

    for (UInt f = 0; f < m_mesh.GetNumFaces(); ++f)
    {
        if (m_faceMask[f] != 1)
        {
            continue;
        }

        const auto numEdges = m_mesh.GetNumFaceEdges(f);

        for (UInt e = 0; e < numEdges; ++e)
        {
            const auto edgeIndex = m_mesh.m_facesEdges[f][e];
            const auto nextEdgeIndex = NextCircularForwardIndex(e, numEdges);
            const auto previousEdgeIndex = NextCircularBackwardIndex(e, numEdges);
            const auto split = m_brotherEdges[edgeIndex] != m_mesh.m_facesEdges[f][nextEdgeIndex] &&
                               m_brotherEdges[edgeIndex] != m_mesh.m_facesEdges[f][previousEdgeIndex];

            if (split)
            {
                m_edgeMask[edgeIndex] = 1;
            }
        }
    }
}

void MeshRefinement::SmoothRefinementMasks()
{
    if (m_meshRefinementParameters.directional_refinement == 1)
    {
        throw AlgorithmError("Directional refinement cannot be used in combination with smoothing. Please set directional refinement to off!");
    }
    if (m_meshRefinementParameters.smoothing_iterations == 0)
    {
        return;
    }

    std::vector splitEdge(m_edgeMask.size(), false);

    for (int iter = 0; iter < m_meshRefinementParameters.smoothing_iterations; ++iter)
    {
        std::fill(splitEdge.begin(), splitEdge.end(), false);

        for (UInt f = 0; f < m_mesh.GetNumFaces(); ++f)
        {
            if (m_faceMask[f] != 1)
            {
                continue;
            }

            const auto numEdges = m_mesh.GetNumFaceEdges(f);
            FindEdgesToSplit(f, numEdges, splitEdge);
        }

        UpdateFaceRefinementMask(splitEdge);
        UpdateEdgeRefinementMask();
    }
}
