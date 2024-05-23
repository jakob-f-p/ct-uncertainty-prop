#include "ObjectPropertyWidget.h"

#include "../Utils/CoordinateRowWidget.h"
#include "../Utils/NameLineEdit.h"
#include "../../Utils/Overload.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>

FloatParameterSpanWidget::FloatParameterSpanWidget(FloatObjectProperty const& property, QWidget* parent) :
        QWidget(parent),
        NameEdit(new NameLineEdit()),
        Current (new QDoubleSpinBox()),
        Min (new QDoubleSpinBox()),
        Step(new QDoubleSpinBox()),
        Max (new QDoubleSpinBox()) {

    auto* fLayout = new QFormLayout(this);

    fLayout->addRow("Name", NameEdit);
    NameEdit->SetData(QString::fromStdString(property.GetName()));
    NameEdit->setEnabled(false);

    fLayout->addRow("Current", Current);
    Current->setValue(property.Get());
    Current->setEnabled(false);

    auto* rangeWidget = new QWidget();
    auto* gLayout = new QGridLayout(rangeWidget);

    auto* minLabel  = new QLabel("Min");
    auto* stepLabel = new QLabel("Step");
    auto* maxLabel  = new QLabel("Max");
    gLayout->addWidget(minLabel, 0, 0);
    gLayout->addWidget(Min, 1, 0);
    gLayout->addWidget(stepLabel, 2, 0);
    gLayout->addWidget(Step, 3, 0);
    gLayout->addWidget(maxLabel, 4, 0);
    gLayout->addWidget(Max, 5, 0);

    fLayout->addRow("Range", rangeWidget);
}

auto FloatParameterSpanWidget::SetParameterSpan(ParameterSpan<float> floatParameterSpan) noexcept -> void {

}

FloatPointParameterSpanWidget::FloatPointParameterSpanWidget(FloatPointObjectProperty const& property,
                                                             QWidget* parent) :
        QWidget(parent),
        NameEdit(new NameLineEdit()),
        Current (new CoordinateRowWidget(false)),
        Min (new CoordinateRowWidget(false)),
        Step(new CoordinateRowWidget(false)),
        Max (new CoordinateRowWidget(false)) {

    auto* fLayout = new QFormLayout(this);

    fLayout->addRow("Name", NameEdit);
    NameEdit->SetData(QString::fromStdString(property.GetName()));
    NameEdit->setEnabled(false);

    fLayout->addRow("Current", Current);
    Current->SetRowData(0, CoordinateRowWidget::RowData(property.Get()));
    Current->setEnabled(false);

    fLayout->addRow("Min", Min);
    fLayout->addRow("Step", Step);
    fLayout->addRow("Max", Max);
}

auto FloatPointParameterSpanWidget::SetParameterSpan(ParameterSpan<FloatPoint> floatParameterSpan) noexcept -> void {

}

ObjectPropertyGroup::ObjectPropertyGroup(ArtifactVariantPointer artifactVariantPointer, QWidget* parent) :
        VLayout(new QVBoxLayout(this)),
        ParameterProperties(artifactVariantPointer.GetProperties()),
        SelectPropertyComboBox([&]() {
            auto* comboBox = new QComboBox();

            for (auto const& propertyVariant: ParameterProperties) {
                auto name = std::visit([](auto const& property) { return property.GetName(); }, propertyVariant);
                comboBox->addItem(QString::fromStdString(name));
            }

            return comboBox;
        }()),
        PropertyWidget({}) {

    setTitle("Property");
}

auto ObjectPropertyGroup::UpdatePropertyWidget() {
    int idx = SelectPropertyComboBox->currentIndex();

    if (idx == -1)
        return;

    std::visit([this](auto* widget) {
        if (!widget)
            return;

        VLayout->removeWidget(widget);

        delete widget;
    }, PropertyWidget);

    auto& propertyVariant = ParameterProperties.at(idx);
    PropertyWidget = std::visit(Overload {
        [](FloatObjectProperty& p) -> PropertyWidgetPointer { return new FloatParameterSpanWidget(p); },
        [](FloatPointObjectProperty& p) -> PropertyWidgetPointer { return new FloatPointParameterSpanWidget(p); },
    }, propertyVariant);

    std::visit([this](auto* widget) { VLayout->addWidget(widget); }, PropertyWidget);
}
