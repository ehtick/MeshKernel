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

#pragma once

#include <MeshKernel/Constants.hpp>
#include <MeshKernel/Entities.hpp>
#include <MeshKernel/Exceptions.hpp>

// include boost
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS

#include "MeshKernel/BoundingBox.hpp"
#include "MeshKernel/Utilities/RTreeBase.hpp"

#include <concepts>
#include <utility>

// r-tree
// https://gist.github.com/logc/10272165

namespace meshkernel
{
    // using a namespace alias
    namespace bg = boost::geometry;
    namespace bgi = boost::geometry::index;

    /// @brief Class used for inquiring adjacent nodes in a mesh.
    ///
    /// The RTree class is designed for querying adjacent nodes within a mesh.
    /// It encapsulates the boost::geometry::index::rtree functionality and extends it with a user-friendly
    /// interface to perform common spatial queries. This class is templated on the projection type,
    /// allowing flexibility in the geometric coordinate system used (default is bg::cs::cartesian).
    ///
    /// @tparam projection The geometric coordinate system projection (default is bg::cs::cartesian).
    ///
    /// The RTree class is primarily utilized within the mesh library for efficiently querying
    /// the closest mesh nodes and edges to a specified point. It employs the RTreeBase class as its base.
    ///
    /// Internally, the RTree class maintains a query cache (`m_queryCache`) which is a vector used
    /// to collect and store query results. This design helps optimize performance by avoiding frequent
    /// reallocations when the number of results changes between queries.
    ///
    /// Example usage:
    /// @code
    /// // Create an RTree instance with the default Cartesian projection.
    /// RTree<> cartesianRTree;
    ///
    /// // Perform a search for the nearest neighbors within a specified distance.
    /// auto resultPoints = cartesianRTree.SearchPoints(queryPoint, searchDistance);
    ///
    /// // Perform a search for the single nearest neighbor.
    /// auto nearestPoint = cartesianRTree.SearchNearestPoint(queryPoint);
    /// @endcode
    ///
    /// Note: For advanced use cases and different geometric coordinate systems, users can provide
    /// a custom projection template parameter when instantiating the RTree class.
    ///
    /// For more details on available query methods, refer to the base class documentation: meshkernel::RTreeBase.
    template <typename projection = bg::cs::cartesian>
    class RTree : public RTreeBase
    {
        using Point2D = bg::model::point<double, 2, projection>; ///< Typedef for Point2D
        using Box2D = bg::model::box<Point2D>;                   ///< Typedef for box of Point2D
        using Value2D = std::pair<Point2D, UInt>;                ///< Typedef of pair of Point2D and UInt
        using RTree2D = bgi::rtree<Value2D, bgi::linear<16>>;    ///< Typedef for a 2D RTree

        /// @brief Ninety degrees
        static constexpr double NinetyDegrees = 90.0;

    public:
        /// @brief Builds the tree from a vector of Points
        /// @param[in] nodes The vector of nodes
        void BuildTree(const std::vector<Point>& nodes) override
        {
            auto convert = [](const Point& p)
            { return Point2D(p.x, p.y); };
            BuildTreeFromVector(nodes, m_points2D, convert);
            m_rtree2D = RTree2D(m_points2D);
        }

        /// @brief Builds the tree from a vector of samples
        /// @param[in] samples The vector of samples
        void BuildTree(const std::vector<Sample>& samples) override
        {
            auto convert = [](const Point& p)
            { return Point2D(p.x, p.y); };
            BuildTreeFromVector(samples, m_points2D, convert);
            m_rtree2D = RTree2D(m_points2D);
        }

        /// @brief Builds the tree from a vector of points within a bounding box
        /// @param[in] nodes The vector of nodes
        /// @param[in] boundingBox The vector bounding box
        void BuildTree(const std::vector<Point>& nodes, const BoundingBox& boundingBox) override
        {
            auto convert = [](const Point& p)
            { return Point2D(p.x, p.y); };
            BuildTreeFromVectorWithinBoundingBox(nodes, m_points2D, convert, boundingBox);
            m_rtree2D = RTree2D(m_points2D);
        }

        /// @brief Builds the tree from a vector of samples within a bounding box
        /// @param[in] samples The vector of samples
        /// @param[in] boundingBox The vector bounding box
        void BuildTree(const std::vector<Sample>& samples, const BoundingBox& boundingBox) override
        {
            auto convert = [](const Point& p)
            { return Point2D(p.x, p.y); };
            BuildTreeFromVectorWithinBoundingBox(samples, m_points2D, convert, boundingBox);
            m_rtree2D = RTree2D(m_points2D);
        }

        /// @brief Finds all nodes in the search radius and stores the results in the query cache, to be inquired later
        /// @param[in] node The node
        /// @param[in] searchRadiusSquared The squared search radius around the node
        void SearchPoints(Point const& node, double searchRadiusSquared) override;

        /// @brief Finds the nearest node in the search radius and stores the results in the query cache, to be inquired later
        /// @param[in] node The node
        /// @param[in] searchRadiusSquared The squared search radius around the node
        void SearchNearestPoint(Point const& node, double searchRadiusSquared) override;

        /// @brief Gets the nearest of all nodes
        /// @param[in] node The node
        void SearchNearestPoint(Point const& node) override;

        /// @brief Deletes a node
        /// @param[in] position The index of the point to remove in m_points2D
        void DeleteNode(UInt position) override;

        /// @brief Determines size of the RTree
        [[nodiscard]] UInt Size() const override { return static_cast<UInt>(m_rtree2D.size()); };

        /// @brief Determines if the RTree is empty
        [[nodiscard]] bool Empty() const override { return m_rtree2D.empty(); }

        /// @brief Gets the size of the query
        [[nodiscard]] UInt GetQueryResultSize() const override { return static_cast<UInt>(m_queryCache.size()); }

        /// @brief Gets the index of a sample in the query
        [[nodiscard]] UInt GetQueryResult(UInt index) const override { return m_queryIndices[index]; }

        /// @brief True if a query has results, false otherwise
        [[nodiscard]] bool HasQueryResults() const override { return !m_queryCache.empty(); }

    private:
        /// @brief Performs a spatial search within a search radius
        /// @param[in] node The reference point for the search.
        /// @param[in] searchRadiusSquared The squared search radius.
        /// @param[in] findNearest If true, finds the nearest point; otherwise, finds all points within the radius.
        void Search(Point const& node, double searchRadiusSquared, bool findNearest);

        RTree2D m_rtree2D;                                ///< The 2D RTree
        std::vector<std::pair<Point2D, UInt>> m_points2D; ///< The points
        std::vector<Value2D> m_queryCache;                ///< The query cache
        std::vector<UInt> m_queryIndices;                 ///< The query indices
        UInt m_queryVectorCapacity = 100;                 ///< Capacity of the query vector
    };

    template <typename projection>
    void RTree<projection>::Search(Point const& node, double searchRadiusSquared, bool findNearest)
    {
        if (Empty())
        {
            throw AlgorithmError("RTree is empty, search cannot be performed");
        }

        m_queryCache.reserve(m_queryVectorCapacity);
        m_queryCache.clear();
        const Point2D nodeSought = Point2D(node.x, node.y);
        const auto searchRadius = std::sqrt(searchRadiusSquared);
        Box2D const box(Point2D(node.x - searchRadius, node.y - searchRadius),
                        Point2D(node.x + searchRadius, node.y + searchRadius));

        auto pointIsNearby = [&nodeSought, &searchRadiusSquared](Value2D const& v)
        { return bg::comparable_distance(v.first, nodeSought) <= searchRadiusSquared; };

        auto atPoleOrInBox = [&nodeSought, &box](Value2D const& v)
        {
            const Point2D& p(v.first);
            return (nodeSought.template get<1>() == NinetyDegrees && p.template get<1>() == NinetyDegrees) ||
                   (nodeSought.template get<1>() == -NinetyDegrees && p.template get<1>() == -NinetyDegrees) ||
                   bg::within(p, box);
        };

        if constexpr (std::is_same<projection, bg::cs::cartesian>::value)
        {
            if (findNearest)
            {
                m_rtree2D.query(bgi::within(box) && bgi::satisfies(pointIsNearby) && bgi::nearest(nodeSought, 1),
                                std::back_inserter(m_queryCache));
            }
            else
            {
                m_rtree2D.query(bgi::within(box) && bgi::satisfies(pointIsNearby),
                                std::back_inserter(m_queryCache));
            }
        }
        else if constexpr (std::is_same<projection, bg::cs::geographic<bg::degree>>::value)
        {
            if (findNearest)
            {
                m_rtree2D.query(bgi::satisfies(atPoleOrInBox) && bgi::satisfies(pointIsNearby) && bgi::nearest(nodeSought, 1),
                                std::back_inserter(m_queryCache));
            }
            else
            {
                m_rtree2D.query(bgi::satisfies(atPoleOrInBox) && bgi::satisfies(pointIsNearby),
                                std::back_inserter(m_queryCache));
            }
        }
        else
        {
            throw ConstraintError("Searching for points has not been implemented for this projection type");
        }

        m_queryIndices.clear();
        if (findNearest && !m_queryCache.empty())
        {
            m_queryIndices.emplace_back(m_queryCache[0].second);
        }
        else
        {
            for (const auto& entry : m_queryCache)
            {
                m_queryIndices.emplace_back(entry.second);
            }
        }
    }

    template <typename projection>
    void RTree<projection>::SearchPoints(Point const& node, double searchRadiusSquared)
    {
        Search(node, searchRadiusSquared, false);
    }

    template <typename projection>
    void RTree<projection>::SearchNearestPoint(Point const& node, double searchRadiusSquared)
    {
        Search(node, searchRadiusSquared, true);
    }

    template <typename projection>
    void RTree<projection>::SearchNearestPoint(Point const& node)
    {
        if (Empty())
        {
            throw AlgorithmError("RTree is empty, search cannot be performed");
        }

        m_queryCache.reserve(m_queryVectorCapacity);
        m_queryCache.clear();
        const Point2D nodeSought = Point2D(node.x, node.y);
        m_rtree2D.query(bgi::nearest(nodeSought, 1), std::back_inserter(m_queryCache));

        if (!m_queryCache.empty())
        {
            m_queryIndices.clear();
            m_queryIndices.emplace_back(m_queryCache[0].second);
        }
    }

    template <typename projection>
    void RTree<projection>::DeleteNode(UInt position)
    {
        if (Empty())
        {
            throw AlgorithmError("RTree is empty, deletion cannot performed");
        }

        if (const auto numberRemoved = m_rtree2D.remove(m_points2D[position]); numberRemoved != 1)
        {
            return;
        }
        m_points2D[position] = {Point2D{constants::missing::doubleValue, constants::missing::doubleValue}, std::numeric_limits<UInt>::max()};
    }

} // namespace meshkernel
