#include "MeshKernel/Definitions.hpp"
#include "MeshKernel/Exceptions.hpp"

meshkernel::Projection meshkernel::GetProjectionValue(const int projection)
{
    static const int Cartesian = static_cast<int>(Projection::cartesian);
    static const int Spherical = static_cast<int>(Projection::spherical);
    static const int Accurate = static_cast<int>(Projection::sphericalAccurate);

    if (projection == Cartesian)
    {
        return Projection::cartesian;
    }
    else if (projection == Spherical)
    {
        return Projection::spherical;
    }
    else if (projection == Accurate)
    {
        return Projection::sphericalAccurate;
    }
    else
    {
        throw ConstraintError("Cannot convert integer value, {}, for projection to projection enumeration value", projection);
    }
}

const std::string& meshkernel::ToString(const Projection projection)
{
    static const std::string Cartesian = "Projection::Cartesian";
    static const std::string Spherical = "Projection::Spherical";
    static const std::string Accurate = "Projection::SphericalAccurate";
    static const std::string Unknown = "UNKNOWN";

    switch (projection)
    {
    case Projection::cartesian:
        return Cartesian;
    case Projection::spherical:
        return Spherical;
    case Projection::sphericalAccurate:
        return Accurate;
    default:
        return Unknown;
    }
}