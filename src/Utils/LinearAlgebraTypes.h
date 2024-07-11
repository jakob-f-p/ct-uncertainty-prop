#pragma once

#include <array>


using DoubleVector = std::array<double, 3>;
using FloatVector = std::array<float, 3>;

using DoublePoint = DoubleVector;
using FloatPoint = FloatVector;

[[nodiscard]] auto static
DoubleToFloatPoint(DoublePoint point) -> FloatPoint { return { static_cast<float>(point[0]),
                                                               static_cast<float>(point[1]),
                                                               static_cast<float>(point[2]) }; }
