#include "SimpleTransform.h"

#include "../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>

#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

#include <utility>

auto SimpleTransform::GetMTime() const noexcept -> vtkMTimeType {
    return Transform->GetMTime();
}

auto SimpleTransform::Modified() noexcept -> void {
    Transform->Modified();
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
    std::copy(doubleMatrix, std::next(doubleMatrix, 12), Matrix.data());
}



SimpleTransformWidget::SimpleTransformWidget() :
        TransformRows(new CoordinateRowWidget(true)){

    TransformRows->AppendCoordinatesRow({ -100.0, 100.0, 2.0, 0.0      }, "Translate");
    TransformRows->AppendCoordinatesRow({    0.0, 360.0, 1.0, 0.0, "°" }, "Rotate");
    TransformRows->AppendCoordinatesRow({  -10.0,  10.0, 0.1, 1.0      }, "Scale");

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(TransformRows);
}

auto SimpleTransformWidget::GetData() noexcept -> SimpleTransformData {
    SimpleTransformData data;

    auto rowData = TransformRows->GetRowData();
    for (int i = 0; i < data.size(); i++)
        data[i] = rowData[i].ToArray();

    return data;
}

auto SimpleTransformWidget::SetData(const SimpleTransformData& data) noexcept -> void {
    for (int i = 0; i < data.size(); i++)
        TransformRows->SetRowData(i, CoordinateRowWidget::RowData(data[i]));
}
