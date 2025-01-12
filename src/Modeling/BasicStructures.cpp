#include "BasicStructures.h"

#include "../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QFormLayout>

#include <vtkMath.h>


auto SphereData::PopulateFromWidget(Widget const* widget) noexcept -> void {
    *this = widget->GetData();
}

auto SphereData::PopulateWidget(Widget const* widget) const noexcept -> void {
    widget->Populate(*this);
}

auto Sphere::AddFunctionData(Data& data) const noexcept -> void {
    data.Radius = Function->GetRadius();
    Function->GetCenter(data.Center.data());
}

auto Sphere::SetFunctionData(const Data& data) const noexcept -> void {
    Function->SetRadius(data.Radius);
    Function->SetCenter(data.Center.data());
}

SphereWidget::SphereWidget() :
        RadiusSpinBox(new QDoubleSpinBox()),
        CenterCoordinateRow(new DoubleCoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 })) {

    auto* fLayout = new QFormLayout(this);

    RadiusSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    RadiusSpinBox->setRange(0.0, 100.0);
    RadiusSpinBox->setSingleStep(1.0);
    fLayout->addRow("Radius", RadiusSpinBox);

    fLayout->addRow("Center", CenterCoordinateRow);
}

auto SphereWidget::GetData() const noexcept -> SphereData {
    return { RadiusSpinBox->value(), CenterCoordinateRow->GetRowData(0).ToArray() };
}

auto SphereWidget::Populate(const SphereData& data) const noexcept -> void {
    RadiusSpinBox->setValue(data.Radius);
    CenterCoordinateRow->SetRowData(0, DoubleCoordinateRowWidget::RowData(data.Center));
}




auto BoxData::PopulateFromWidget(Widget const* widget) noexcept -> void {
    *this = widget->GetData();
}

auto BoxData::PopulateWidget(Widget const* widget) const noexcept -> void {
    widget->Populate(*this);
}

auto Box::AddFunctionData(Data& data) const noexcept -> void {
    Function->GetXMin(data.MinPoint.data());
    Function->GetXMax(data.MaxPoint.data());
}

auto Box::SetFunctionData(Data const& data) const noexcept -> void {
    Point minPoint {};
    Point maxPoint {};

    std::ranges::copy(data.MinPoint, minPoint.begin());
    std::ranges::copy(data.MaxPoint, maxPoint.begin());

    Function->SetXMin(minPoint.data());
    Function->SetXMax(maxPoint.data());
}

BoxWidget::BoxWidget() :
        MinMaxPointWidget(new DoubleCoordinateRowWidget(true)) {

    MinMaxPointWidget->AppendCoordinatesRow({ -100.0, 100.0, 1.0, -10.0 }, "Min Point");
    MinMaxPointWidget->AppendCoordinatesRow({ -100.0, 100.0, 1.0,  10.0 }, "Max Point");

    auto* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(MinMaxPointWidget);
}

auto BoxWidget::GetData() const noexcept -> BoxData {
    return { MinMaxPointWidget->GetRowData(0).ToArray(),
             MinMaxPointWidget->GetRowData(1).ToArray() };
}

auto BoxWidget::Populate(const BoxData& data) const noexcept -> void {
    MinMaxPointWidget->SetRowData(0, DoubleCoordinateRowWidget::RowData(data.MinPoint));
    MinMaxPointWidget->SetRowData(1, DoubleCoordinateRowWidget::RowData(data.MaxPoint));
}




auto ConeData::PopulateFromWidget(Widget const* widget) noexcept -> void {
    *this = widget->GetData();
}

auto ConeData::PopulateWidget(Widget const* widget) const noexcept -> void {
    widget->Populate(*this);
}

ConeWidget::ConeWidget() :
        RadiusSpinBox(new QDoubleSpinBox()),
        HeightSpinBox(new QDoubleSpinBox()) {

    auto* fLayout = new QFormLayout(this);

    RadiusSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    RadiusSpinBox->setRange(0.0, 100.0);
    RadiusSpinBox->setSingleStep(1.0);
    fLayout->addRow("Radius", RadiusSpinBox);

    HeightSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    HeightSpinBox->setRange(0.0, 100.0);
    HeightSpinBox->setSingleStep(1.0);
    fLayout->addRow("Height", HeightSpinBox);
}

auto ConeWidget::GetData() const noexcept -> ConeData {
    return { RadiusSpinBox->value(), HeightSpinBox->value() };
}

auto ConeWidget::Populate(ConeData const& data) const noexcept -> void {
    RadiusSpinBox->setValue(data.Radius);
    HeightSpinBox->setValue(data.Height);
}

Cone::Cone() {
    double static constexpr defaultRadius = 5.0;
    double static constexpr defaultHeight = 10.0;
    double static const defaultAngleDeg = vtkMath::DegreesFromRadians(atan(defaultRadius / defaultHeight));

    UnboundedCone->SetAngle(defaultAngleDeg);
    TipPlane->SetNormal(0.0, 0.0, -1.0);
    TipPlane->SetOrigin(0.0, 0.0, 0.0);
    BasePlane->SetNormal(0.0, 0.0, 1.0);
    BasePlane->SetOrigin(0.0, 0.0, defaultHeight);

    ConeFunction->SetOperationTypeToIntersection();
    ConeFunction->AddFunction(UnboundedCone);
    ConeFunction->AddFunction(TipPlane);
    ConeFunction->AddFunction(BasePlane);
}

auto Cone::AddFunctionData(Data& data) const noexcept -> void {
    double const angleRad = vtkMath::RadiansFromDegrees(UnboundedCone->GetAngle());
    double const height = BasePlane->GetOrigin()[2];

    data.Radius = height * tan(angleRad);
    data.Height = height;
}

auto Cone::SetFunctionData(Data const& data) const noexcept -> void {
    double const angleRad = vtkMath::DegreesFromRadians(atan(data.Radius / data.Height));

    UnboundedCone->SetAngle(angleRad);
    BasePlane->SetOrigin(0.0, 0.0, data.Height);
}



auto CylinderData::PopulateFromWidget(Widget const* widget) noexcept -> void {
    *this = widget->GetData();
}

auto CylinderData::PopulateWidget(Widget const* widget) const noexcept -> void {
    widget->Populate(*this);
}


CylinderWidget::CylinderWidget() :
        RadiusSpinBox(new QDoubleSpinBox()),
        HeightSpinBox(new QDoubleSpinBox()) {

    auto* fLayout = new QFormLayout(this);

    RadiusSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    RadiusSpinBox->setRange(0.0, 100.0);
    RadiusSpinBox->setSingleStep(1.0);
    fLayout->addRow("Radius", RadiusSpinBox);

    HeightSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    HeightSpinBox->setRange(0.0, 100.0);
    HeightSpinBox->setSingleStep(1.0);
    fLayout->addRow("Height", HeightSpinBox);
}

auto CylinderWidget::GetData() const noexcept -> CylinderData {
    return { RadiusSpinBox->value(), HeightSpinBox->value() };
}


auto CylinderWidget::Populate(CylinderData const& data) const noexcept -> void {
    RadiusSpinBox->setValue(data.Radius);
    HeightSpinBox->setValue(data.Height);
}



Cylinder::Cylinder() {
    double static constexpr defaultRadius = 5.0;
    double static constexpr defaultHeight = 10.0;

    UnboundedCylinder->SetAxis(0.0, 0.0, 1.0);
    UnboundedCylinder->SetRadius(defaultRadius);
    BottomPlane->SetNormal(0.0, 0.0, -1.0);
    BottomPlane->SetOrigin(0.0, 0.0, 0.0);
    TopPlane->SetNormal(0.0, 0.0, 1.0);
    TopPlane->SetOrigin(0.0, 0.0, defaultHeight);

    CylinderFunction->SetOperationTypeToIntersection();
    CylinderFunction->AddFunction(UnboundedCylinder);
    CylinderFunction->AddFunction(BottomPlane);
    CylinderFunction->AddFunction(TopPlane);
}

auto Cylinder::AddFunctionData(Data& data) const noexcept -> void {
    data.Radius = UnboundedCylinder->GetRadius();
    data.Height = TopPlane->GetOrigin()[2];
}

auto Cylinder::SetFunctionData(Data const& data) const noexcept -> void {
    UnboundedCylinder->SetRadius(data.Radius);
    TopPlane->GetOrigin()[2] = data.Height;
}
