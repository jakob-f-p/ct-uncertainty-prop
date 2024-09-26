#pragma once

#include <vtkImplicitFunction.h>


class ImplicitCone : public vtkImplicitFunction {
public:
    ImplicitCone(const ImplicitCone&) = delete;
    void operator=(const ImplicitCone&) = delete;

    static ImplicitCone* New();
    vtkTypeMacro(ImplicitCone, vtkImplicitFunction);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    using vtkImplicitFunction::EvaluateFunction;
    double EvaluateFunction(double x[3]) override;

    void EvaluateGradient(double x[3], double g[3]) override;

    auto
    SetAngle(double angleDeg) -> void;

    [[nodiscard]] auto
    GetAngle() const noexcept -> double;

protected:
    ImplicitCone();
    ~ImplicitCone() override = default;

private:
    double Angle;
};
