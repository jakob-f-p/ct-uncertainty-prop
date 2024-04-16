#include "BasicStructures.h"

#include <QDoubleSpinBox>
#include <QFormLayout>

auto SphereDataImpl::AddFunctionWidget(QFormLayout* fLayout) noexcept -> void {
    auto* radiusSpinBox = new QDoubleSpinBox();
    radiusSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    radiusSpinBox->setObjectName(SphereRadiusSpinBoxName);
    radiusSpinBox->setRange(0.0, 100.0);
    radiusSpinBox->setSingleStep(1.0);
    fLayout->addRow("Radius", radiusSpinBox);

    auto* centerCoordinates = CtStructure::GetCoordinatesRow(SphereCenterName, -100.0, 100.0, 1.0);
    fLayout->addRow("Center", centerCoordinates);
}

auto SphereDataImpl::PopulateFromWidget(QWidget* widget) noexcept -> void {
    auto* radiusSpinBox = widget->findChild<QDoubleSpinBox*>(SphereRadiusSpinBoxName);
    Radius = radiusSpinBox->value();
    for (int i = 0; i <Center.size(); i++) {
        auto* centerSpinBox = widget->findChild<QDoubleSpinBox*>(
                CtStructure::GetAxisSpinBoxName(SphereCenterName, CtStructure::AxisNames[i]));
        Center.at(i) = centerSpinBox->value();
    }
}

auto SphereDataImpl::PopulateWidget(QWidget* widget) const noexcept -> void {
    auto* radiusSpinBox = widget->findChild<QDoubleSpinBox*>(SphereRadiusSpinBoxName);
    radiusSpinBox->setValue(Radius);
    for (int i = 0; i <Center.size(); i++) {
        auto* centerSpinBox = widget->findChild<QDoubleSpinBox*>(
                CtStructure::GetAxisSpinBoxName(SphereCenterName, CtStructure::AxisNames[i]));
        centerSpinBox->setValue(Center.at(i));
    }
}

auto SphereStructureImpl::AddFunctionData(Data& data) const noexcept -> void {
    data.Radius = Function->GetRadius();
    Function->GetCenter(data.Center.data());
}

auto SphereStructureImpl::SetFunctionData(const Data& data) noexcept -> void {
    Function->SetRadius(data.Radius);
    Function->SetCenter(data.Center.data());
}

const QString SphereStructureImpl::Data::SphereRadiusSpinBoxName = "SphereRadius";
const QString SphereStructureImpl::Data::SphereCenterName = "SphereCenter";



auto BoxDataImpl::AddFunctionWidget(QFormLayout* fLayout) noexcept -> void {
    QWidget* minPointCoordinates = CtStructure::GetCoordinatesRow(BoxMinPointName, -100.0, 100.0, 1.0);
    QWidget* maxPointCoordinates = CtStructure::GetCoordinatesRow(BoxMaxPointName, -100.0, 100.0, 1.0);

    fLayout->addRow("Min. Point", minPointCoordinates);
    fLayout->addRow("Max. Point", maxPointCoordinates);
}

auto BoxDataImpl::PopulateFromWidget(QWidget* widget) noexcept -> void {
    for (int i = 0; i <MinPoint.size(); i++) {
        auto* minPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                CtStructure::GetAxisSpinBoxName(BoxMinPointName, CtStructure::AxisNames[i]));
        MinPoint[i] = minPointSpinBox->value();

        auto* maxPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                CtStructure::GetAxisSpinBoxName(BoxMaxPointName, CtStructure::AxisNames[i]));
        MaxPoint[i] = maxPointSpinBox->value();
    }
}

auto BoxDataImpl::PopulateWidget(QWidget* widget) const noexcept -> void {
    for (int i = 0; i < MinPoint.size(); i++) {
        auto* minPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                CtStructure::GetAxisSpinBoxName(BoxMinPointName, CtStructure::AxisNames[i]));
        minPointSpinBox->setValue(MinPoint[i]);

        auto* maxPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                CtStructure::GetAxisSpinBoxName(BoxMaxPointName, CtStructure::AxisNames[i]));
        maxPointSpinBox->setValue(MaxPoint[i]);
    }
}

auto BoxStructureImpl::AddFunctionData(Data& data) const noexcept -> void {
    Function->GetXMin(data.MinPoint.data());
    Function->GetXMax(data.MaxPoint.data());
}

auto BoxStructureImpl::SetFunctionData(const Data& data) noexcept -> void {
    Point minPoint {};
    Point maxPoint {};

    std::copy(data.MinPoint.begin(), data.MinPoint.end(), minPoint.begin());
    std::copy(data.MaxPoint.begin(), data.MaxPoint.end(), maxPoint.begin());

    Function->SetXMin(minPoint.data());
    Function->SetXMax(maxPoint.data());
}

const QString BoxStructureImpl::Data::BoxMinPointName = "BoxMinPoint";
const QString BoxStructureImpl::Data::BoxMaxPointName = "BoxMaxPoint";
