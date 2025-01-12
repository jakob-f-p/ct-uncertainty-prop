#include "SimpleTransform.h"

#include "../Ui/Utils/CoordinateRowWidget.h"

#include <QGridLayout>
#include <QLabel>

#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

#include <utility>

SimpleTransform::SimpleTransform(SimpleTransform const& other) :
        TranslationValues(other.TranslationValues),
        RotationAngles(other.RotationAngles),
        ScaleFactors(other.ScaleFactors),
        Matrix(other.Matrix),
        InverseMatrix(other.Matrix) {}

auto SimpleTransform::GetMTime() const noexcept -> vtkMTimeType {
    return Transform->GetMTime();
}

auto SimpleTransform::Modified() const noexcept -> void {
    Transform->Modified();
}

auto SimpleTransform::GetData() const noexcept -> SimpleTransformData {
    return { TranslationValues, RotationAngles, ScaleFactors };
}

auto SimpleTransform::SetData(const SimpleTransformData& transformData) noexcept -> void {
    TranslationValues = transformData[0];
    RotationAngles =    transformData[1];
    ScaleFactors =      transformData[2];

    InverseTransform->Identity();
    InverseTransform->Translate(TranslationValues.data());
    InverseTransform->RotateX(RotationAngles[0]);
    InverseTransform->RotateY(RotationAngles[1]);
    InverseTransform->RotateZ(RotationAngles[2]);
    InverseTransform->Scale(ScaleFactors.data());

    Transform->DeepCopy(InverseTransform);
    Transform->Inverse();

    InverseTransform->Update();
    Transform->Update();

    vtkNew<vtkMatrix4x4> const vtkMatrix;
    InverseTransform->GetMatrix(vtkMatrix);
    double const* doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), InverseMatrix.data());
    Transform->GetMatrix(vtkMatrix);
    doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), Matrix.data());
}

auto SimpleTransform::LeftMultiply(SimpleTransform const& other) const noexcept -> SimpleTransform {
    SimpleTransform transform {};

    transform.Transform->Identity();
    transform.Transform->PreMultiply();
    transform.Transform->Concatenate(Transform);
    transform.Transform->Concatenate(other.Transform);

    transform.InverseTransform->DeepCopy(transform.Transform);
    transform.InverseTransform->Inverse();

    Transform->Update();
    InverseTransform->Update();

    vtkNew<vtkMatrix4x4> const vtkMatrix;
    Transform->GetMatrix(vtkMatrix);
    double const* doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), transform.Matrix.data());
    InverseTransform->GetMatrix(vtkMatrix);
    doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), transform.InverseMatrix.data());

    return transform;
}

auto SimpleTransform::RightMultiply(SimpleTransform const& other) const noexcept -> SimpleTransform {
    SimpleTransform transform {};

    transform.Transform->Identity();
    transform.Transform->PostMultiply();
    transform.Transform->Concatenate(Transform);
    transform.Transform->Concatenate(other.Transform);

    transform.InverseTransform->DeepCopy(transform.Transform);
    transform.InverseTransform->Inverse();

    Transform->Update();
    InverseTransform->Update();

    vtkNew<vtkMatrix4x4> const vtkMatrix;
    Transform->GetMatrix(vtkMatrix);
    const double* doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), transform.Matrix.data());
    InverseTransform->GetMatrix(vtkMatrix);
    doubleMatrix = vtkMatrix->GetData();
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), transform.InverseMatrix.data());

    return transform;
}



SimpleTransformWidget::SimpleTransformWidget() :
        TransformRows(new DoubleCoordinateRowWidget(true)){

    TransformRows->AppendCoordinatesRow({ -100.0, 100.0, 2.0, 0.0      }, "Translate");
    TransformRows->AppendCoordinatesRow({    0.0, 360.0, 1.0, 0.0, 1, "°" }, "Rotate");
    TransformRows->AppendCoordinatesRow({  -10.0,  10.0, 0.1, 1.0      }, "Scale");

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(TransformRows);
}

auto SimpleTransformWidget::GetData() const noexcept -> SimpleTransformData {
    SimpleTransformData data;

    auto const rowData = TransformRows->GetRowData();
    for (int i = 0; i < data.size(); i++)
        data[i] = rowData[i].ToArray();

    return data;
}

auto SimpleTransformWidget::SetData(const SimpleTransformData& data) const noexcept -> void {
    for (int i = 0; i < data.size(); i++)
        TransformRows->SetRowData(i, DoubleCoordinateRowWidget::RowData(data[i]));
}
