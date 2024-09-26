#pragma once

#include <vtkImplicitFunction.h>

#include <array>


class ImplicitCylinder : public vtkImplicitFunction {
public:
    ImplicitCylinder(const ImplicitCylinder&) = delete;
    void operator=(const ImplicitCylinder&) = delete;

    vtkTypeMacro(ImplicitCylinder, vtkImplicitFunction);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static ImplicitCylinder* New();

    using vtkImplicitFunction::EvaluateFunction;
    double EvaluateFunction(double x[3]) override;

    void EvaluateGradient(double x[3], double g[3]) override;

    vtkSetMacro(Radius, double);
    vtkGetMacro(Radius, double);

    auto
    SetCenter(std::array<double, 3> center) -> void;

    [[nodiscard]] auto
    GetCenter() const noexcept -> double;

protected:
    ImplicitCylinder();
    ~ImplicitCylinder() override = default;

private:
    double Radius;
    std::array<double, 3> Center;
    std::array<double, 3> Axis;
};
