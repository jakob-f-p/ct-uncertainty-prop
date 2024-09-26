#include "ImplicitCylinder.h"

#include <vtkMath.h>
#include <vtkObjectFactory.h>

#include <format>


vtkStandardNewMacro(ImplicitCylinder);

ImplicitCylinder::ImplicitCylinder() :
        Center({ 0.0, 0.0, 0.0 }),
        Axis({ 0.0, 0.0, 1.0 }),
        Radius(1.0) {}

auto ImplicitCylinder::EvaluateFunction(double x[3]) -> double {
    std::array<double, 3> const xCenterDiff { x[0] - Center[0],
                                              x[1] - Center[1],
                                              x[2] - Center[2] };

    // projection onto cylinder axis
    double const proj = vtkMath::Dot(Axis, xCenterDiff);

    // return distance^2 - R^2
    return (vtkMath::Dot(xCenterDiff, xCenterDiff) - proj * proj) - Radius * Radius;
}

void ImplicitCylinder::EvaluateGradient(double x[3], double g[3]) {
    // Determine the radial vector from the point x to the line. This
    // means finding the closest point to the line. Get parametric
    // location along cylinder axis. Remember Axis is normalized.
    double const t = Axis[0] * (x[0] - Center[0])
            + Axis[1] * (x[1] - Center[1])
            + Axis[2] * (x[2] - Center[2]);

    // Compute closest point
    std::array<double, 3> const cp { Center[0] + t * Axis[0],
                                     Center[1] + t * Axis[1],
                                     Center[2] + t * Axis[2] };

    // Gradient is 2*r. Project onto x-y-z axes.
    g[0] = 2.0 * (x[0] - cp[0]);
    g[1] = 2.0 * (x[1] - cp[1]);
    g[2] = 2.0 * (x[2] - cp[2]);
}

void ImplicitCylinder::PrintSelf(ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << std::format("Center: ({}, {}, {})", Center[0], Center[1], Center[2]);
    os << indent << std::format("Axis: ({}, {}, {})", Axis[0], Axis[1], Axis[2]);
    os << indent << "Radius: " << Radius << "\n";
}
