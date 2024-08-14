#pragma once

#include <array>
#include <limits>


using DoubleVector = std::array<double, 3>;
using FloatVector = std::array<float, 3>;

using DoublePoint = DoubleVector;
using FloatPoint = FloatVector;

[[nodiscard]] auto static
DoubleToFloatPoint(DoublePoint point) -> FloatPoint { return { static_cast<float>(point[0]),
                                                               static_cast<float>(point[1]),
                                                               static_cast<float>(point[2]) }; }

[[nodiscard]] auto static
VectorLength(DoubleVector const& v) -> double {
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

[[nodiscard]] auto static
VectorDistance(DoubleVector const& a, DoubleVector const& b) -> double {
    return VectorLength({ a[0] - b[0], a[1] - b[1], a[2] - b[2] });
}
