﻿//---- GPL ---------------------------------------------------------------------
//
// Copyright (C)  Stichting Deltares, 2011-2023.
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

#include <MeshKernel/AveragingStrategies/AveragingStrategy.hpp>
#include <MeshKernel/Entities.hpp>

namespace meshkernel::averaging
{
    /// @brief ClosestAveragingStrategy implements the AveragingStrategy which selects the value of the closest added point.
    class ClosestAveragingStrategy final : public AveragingStrategy
    {
    public:
        /// @brief Construct a new ClosestAveragingStrategy.
        /// @param[in] interpolationPoint The point for which the average should be calculated.
        /// @param[in] projection         The projection used to calculate distances with.
        explicit ClosestAveragingStrategy(Projection projection);

        /// @brief Calculates the average value based on the sample values.
        /// @param[in] interpolationPoint The point for which the average should be calculated.
        /// @param[in] samples The sample points and values used by this strategy.
        /// @return The calculated average
        [[nodiscard]] double Calculate(const Point& interpolationPoint,
                                       const std::vector<Sample>& samples) const override;

    private:
        /// @brief The projection used to calculate the squared distance.
        Projection const m_projection;
    };
} // namespace meshkernel::averaging
