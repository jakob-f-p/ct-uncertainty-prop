#include "ImplicitCone.h"

#include <vtkMath.h>
#include <vtkObjectFactory.h>

#include <numbers>


vtkStandardNewMacro(ImplicitCone);

ImplicitCone::ImplicitCone() :
    Angle(std::numbers::pi / 4) {}

auto ImplicitCone::SetAngle(double angleDeg) -> void {
    if (angleDeg <= 0.0 || angleDeg >= 90.0)
        throw std::runtime_error("invalid angle");

    Angle = vtkMath::RadiansFromDegrees(Angle);

    Modified();
}

auto ImplicitCone::GetAngle() const noexcept -> double { return vtkMath::DegreesFromRadians(Angle); }

auto ImplicitCone::EvaluateFunction(double x[3]) -> double {
    double const tanTheta = tan(vtkMath::RadiansFromDegrees(Angle));
    return x[0] * x[0] + x[1] * x[1] - x[2] * x[2] * tanTheta * tanTheta;
}

void ImplicitCone::EvaluateGradient(double x[3], double g[3]) {
    double const tanTheta = tan(vtkMath::RadiansFromDegrees(Angle));
    g[0] = -2.0 * x[0] * tanTheta * tanTheta;
    g[1] = 2.0 * x[1];
    g[2] = 2.0 * x[2];
}

void ImplicitCone::PrintSelf(ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Angle: " << Angle << "\n";
}
