//---- GPL ---------------------------------------------------------------------
//
// Copyright (C)  Stichting Deltares, 2011-2024.
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

#include <cstring>
#include <vector>

#include "MeshKernelApi/ApiCache/CachedPointValues.hpp"

namespace meshkernelapi
{

    /// @brief Cache boundary polygon points
    class MeshBoundariesAsPolygonCache : public CachedPointValues
    {
    public:
        /// @brief Constructor
        MeshBoundariesAsPolygonCache(const std::vector<meshkernel::Point>& selectionPolygonPoints,
                                     const std::vector<meshkernel::Point>& boundaryPoints);

        /// @brief Determine if current options match those used to construct the object
        bool ValidOptions(const std::vector<meshkernel::Point>& selectionPolygonPoints) const;

    private:
        const std::vector<meshkernel::Point> m_selectionPolygonPoints; ///< the points of the selection polygon
    };

} // namespace meshkernelapi
