#include "BasicStructures.h"

#include "../Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QFormLayout>

auto SphereData::PopulateFromWidget(Widget* widget) noexcept -> void {
    *this = widget->GetData();
}

auto SphereData::PopulateWidget(Widget* widget) const noexcept -> void {
    widget->Populate(*this);
}

auto Sphere::AddFunctionData(Data& data) const noexcept -> void {
    data.Radius = Function->GetRadius();
    Function->GetCenter(data.Center.data());
}

auto Sphere::SetFunctionData(const Data& data) noexcept -> void {
    Function->SetRadius(data.Radius);
    Function->SetCenter(data.Center.data());
}

SphereWidget::SphereWidget() :
        RadiusSpinBox(new QDoubleSpinBox()),
        CenterCoordinateRow(new CoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 })) {

    auto* fLayout = new QFormLayout(this);

    RadiusSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    RadiusSpinBox->setRange(0.0, 100.0);
    RadiusSpinBox->setSingleStep(1.0);
    fLayout->addRow("Radius", RadiusSpinBox);

    fLayout->addRow("Center", CenterCoordinateRow);
}

auto SphereWidget::GetData() noexcept -> SphereData {
    SphereData data;

    data.Radius = RadiusSpinBox->value();
    data.Center = CenterCoordinateRow->GetRowData(0).ToArray();

    return data;
}

auto SphereWidget::Populate(const SphereData& data) noexcept -> void {
    RadiusSpinBox->setValue(data.Radius);
    CenterCoordinateRow->SetRowData(0, CoordinateRowWidget::RowData(data.Center));
}




auto BoxData::PopulateFromWidget(Widget* widget) noexcept -> void {
    *this = widget->GetData();
}

auto BoxData::PopulateWidget(Widget* widget) const noexcept -> void {
    widget->Populate(*this);
}

auto Box::AddFunctionData(Data& data) const noexcept -> void {
    Function->GetXMin(data.MinPoint.data());
    Function->GetXMax(data.MaxPoint.data());
}

auto Box::SetFunctionData(const Data& data) noexcept -> void {
    Point minPoint {};
    Point maxPoint {};

    std::copy(data.MinPoint.begin(), data.MinPoint.end(), minPoint.begin());
    std::copy(data.MaxPoint.begin(), data.MaxPoint.end(), maxPoint.begin());

    Function->SetXMin(minPoint.data());
    Function->SetXMax(maxPoint.data());
}

BoxWidget::BoxWidget() :
        MinMaxPointWidget(new CoordinateRowWidget(true)) {

    MinMaxPointWidget->AppendCoordinatesRow({ -100.0, 100.0, 1.0, -10.0 }, "Min Point");
    MinMaxPointWidget->AppendCoordinatesRow({ -100.0, 100.0, 1.0,  10.0 }, "Max Point");

    auto* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(MinMaxPointWidget);
}

auto BoxWidget::GetData() noexcept -> BoxData {
    BoxData data {};

    data.MinPoint = MinMaxPointWidget->GetRowData(0).ToArray();
    data.MaxPoint = MinMaxPointWidget->GetRowData(1).ToArray();

    return data;
}

auto BoxWidget::Populate(const BoxData& data) noexcept -> void {
    MinMaxPointWidget->SetRowData(0, CoordinateRowWidget::RowData(data.MinPoint));
    MinMaxPointWidget->SetRowData(1, CoordinateRowWidget::RowData(data.MaxPoint));
}
