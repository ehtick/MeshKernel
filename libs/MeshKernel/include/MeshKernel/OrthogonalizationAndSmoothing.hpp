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

#include <MeshKernel/Definitions.hpp>
#include <MeshKernel/LandBoundaries.hpp>
#include <MeshKernel/Orthogonalizer.hpp>
#include <MeshKernel/Parameters.hpp>
#include <MeshKernel/Smoother.hpp>
#include <MeshKernel/UndoActions/UndoAction.hpp>

namespace meshkernel
{
    // Forward declare everything to reduce compile time dependency
    class Point;
    class Mesh2D;
    class Polygons;
    enum class Projection;

    /// @brief Orthogonalizion (optimize the aspect ratios) and mesh smoothing (optimize internal face angles or area).
    ///
    /// This class implements the mesh orthogonalization and smoothing algorithm
    /// as described in D-Flow FM technical manual (consult this manual for the
    /// mathematical details on the equations). The algorithm operates on mesh2d and
    /// is composed of two differential equations: the first equation maximizes orthogonalization
    /// between edges and flow edges and the second equation reduces the
    /// differences of the internal mesh angles (mesh smoothness). For this
    /// reason, the OrthogonalizationAndSmoothing class is composed of a
    /// smoother and an orthogonalizer, where the nodal contributions are
    /// computed by separate classes. Essentially, the algorithm executes the following steps:
    ///
    /// -   An initialization step: The original mesh boundaries are saved. In
    ///     case the mesh needs to be snapped to the land boundaries, the indices of the land boundaries
    ///     are mapped to the boundary mesh edges (`LandBoundaries::FindNearestMeshBoundary`).
    ///
    /// -   An outer loop, which itself is composed of the following parts:
    ///
    ///     1.  Computation of the orthogonalizer contributions.
    ///
    ///     2.  Computation of the smoother contributions.
    ///
    ///     3.  Allocation of the linear system to be solved.
    ///
    ///     4.  Summation of the two contributions (matrix assembly). The two
    ///         contributions are weighted based on the desired smoothing to
    ///         orthogonality ratio. OpenMP parallelization is used when
    ///         summing the terms (loop iterations are independent).
    ///
    /// -   An inner iteration: the resulting linear system is solved
    ///     explicitly. The nodal coordinates are updated and the nodes moving
    ///     on the mesh boundary are projected to the original mesh boundary
    ///     (`OrthogonalizationAndSmoothing::SnapMeshToOriginalMeshBoundary`).
    ///     In case a projection to land boundary is requested, the mesh nodes are projected to the land
    ///     boundaries. An OpenMP parallelization is used in
    ///     `OrthogonalizationAndSmoothing::Solve` because the update
    ///     of the nodal coordinates is made iteration-independent.
    class OrthogonalizationAndSmoothing
    {

    public:
        /// Set the parameters
        /// @param[in] mesh The mesh to orthogonalize
        /// @param[in] polygon The polygon where orthogonalization should occur
        /// @param[in] landBoundaries The land boundaries
        /// @param[in] projectToLandBoundaryOption Snap to land boundaries (1) or not (0)
        /// @param[in] orthogonalizationParameters The orthogonalization parameters
        OrthogonalizationAndSmoothing(Mesh2D& mesh,
                                      std::unique_ptr<Polygons> polygon,
                                      std::unique_ptr<LandBoundaries> landBoundaries,
                                      LandBoundaries::ProjectToLandBoundaryOption projectToLandBoundaryOption,
                                      const OrthogonalizationParameters& orthogonalizationParameters);

        /// @brief Initializes the object
        [[nodiscard]] std::unique_ptr<UndoAction> Initialize();

        /// @brief Executes the entire algorithm
        void Compute();

        /// @brief Prepares the outer iteration, calculates orthogonalizer and smoother coefficients and assable the linear system
        void PrepareOuterIteration();

        /// @brief Performs an inner iteration, update the mesh node positions
        void Solve();

        /// @brief Finalize the outer iteration, computes new mu and face areas, masscenters, circumcenters
        void FinalizeOuterIteration();

    private:
        /// @brief Get the node type
        MeshNodeType GetNodeType(const UInt nodeId) const { return m_nodesTypes[nodeId]; }

        /// @brief Find the id's of the neighbouring boundary nodes.
        void FindNeighbouringBoundaryNodes(const UInt nodeId,
                                           const UInt nearestPointIndex,
                                           UInt& leftNode,
                                           UInt& rightNode) const;

        /// @brief Project mesh nodes back to the original mesh boundary (orthonet_project_on_boundary)
        void SnapMeshToOriginalMeshBoundary();

        /// @brief Assembles the contributions of smoother and orthogonalizer
        void ComputeLinearSystemTerms();

        /// Computes how much the coordinates of a node need to be incremented at each inner iteration.
        /// @param[in] nodeIndex The node index
        /// @param[out] dx0 The computed x increment
        /// @param[out] dy0 The computed y increment
        /// @param[out] weightsSum The sum of the weights in x and y
        void ComputeLocalIncrements(UInt nodeIndex,
                                    double& dx0,
                                    double& dy0,
                                    std::array<double, 2>& weightsSum);

        /// @brief Update the nodal coordinates based on the increments
        /// @param[in] nodeIndex
        void UpdateNodeCoordinates(UInt nodeIndex);

        /// @brief Allocate linear system vectors
        void AllocateLinearSystem();

        /// @brief Compute nodes local coordinates (comp_local_coords)
        void ComputeCoordinates() const;

        std::vector<std::vector<UInt>> m_nodesNodes; ///< Node to node connectivity
        std::vector<MeshNodeType> m_nodesTypes;      ///< The node types

        Mesh2D& m_mesh;                                                            ///< A reference to mesh
        Smoother m_smoother;                                                       ///< The smoother
        Orthogonalizer m_orthogonalizer;                                           ///< The orthogonalizer
        std::unique_ptr<Polygons> m_polygons;                                      ///< The polygon pointer where to perform the orthogonalization
        std::unique_ptr<LandBoundaries> m_landBoundaries;                          ///< The land boundaries pointer
        LandBoundaries::ProjectToLandBoundaryOption m_projectToLandBoundaryOption; ///< The project to land boundary option
        OrthogonalizationParameters m_orthogonalizationParameters;                 ///< The orthogonalization parameters

        std::vector<UInt> m_localCoordinatesIndices; ///< Used in sphericalAccurate projection (iloc)
        std::vector<Point> m_localCoordinates;       ///< Used in sphericalAccurate projection (xloc,yloc)
        std::vector<Point> m_orthogonalCoordinates;  ///< A copy of the mesh node, orthogonalized
        std::vector<Point> m_originalNodes;          ///< The original mesh

        // Linear system terms
        std::vector<UInt> m_compressedEndNodeIndex;   ///< Start index in m_compressedWeightX
        std::vector<UInt> m_compressedStartNodeIndex; ///< End index in m_compressedWeightY
        std::vector<double> m_compressedWeightX;      ///< The computed weights X
        std::vector<double> m_compressedWeightY;      ///< The computed weights Y
        std::vector<double> m_compressedRhs;          ///< The right hand side
        std::vector<UInt> m_compressedNodesNodes;     ///< The indices of the neighbouring nodes

        // run-time parameters
        double m_mumax = 0.0; ///< Mumax stored for runtime
        double m_mu = 0.0;    ///< Mu stored for runtime
    };
} // namespace meshkernel
