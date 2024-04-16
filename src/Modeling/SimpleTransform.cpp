#include "SimpleTransform.h"

#include <vtkMatrix4x4.h>

auto SimpleTransform::GetMTime() const noexcept -> vtkMTimeType {
    return Transform->GetMTime();
}

auto SimpleTransform::Modified() noexcept -> void {
    return Transform->Modified();
}

auto SimpleTransform::GetData() const noexcept -> SimpleTransformData {
    return { TranslationValues, RotationAngles, ScaleFactors };
}

auto SimpleTransform::SetData(const SimpleTransformData& transformData) noexcept -> void {
    TranslationValues = transformData[0];
    RotationAngles =    transformData[1];
    ScaleFactors =      transformData[2];

    Transform->Identity();
    Transform->Translate(TranslationValues.data());
    Transform->RotateX(RotationAngles[0]);
    Transform->RotateY(RotationAngles[1]);
    Transform->RotateZ(RotationAngles[2]);
    Transform->Scale(ScaleFactors.data());
    Transform->Inverse();

    Transform->Update();

    vtkNew<vtkMatrix4x4> vtkMatrix;
    Transform->GetMatrix(vtkMatrix);

    const double* doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, N4x4), Matrix.data()->data());
}
