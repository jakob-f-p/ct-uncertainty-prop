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
        Property(property),
        Current (new QDoubleSpinBox()),
        Min (new QDoubleSpinBox()),
        Max (new QDoubleSpinBox()),
        Step(new QDoubleSpinBox()) {

    auto* fLayout = new QFormLayout(this);

    fLayout->addRow("Current", Current);
    Current->setEnabled(false);

    auto* rangeWidget = new QWidget();
    auto* gLayout = new QGridLayout(rangeWidget);
    gLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->setAlignment(Qt::AlignVCenter);

    auto* minLabel  = new QLabel("Min");
    auto* stepLabel = new QLabel("Step");
    auto* maxLabel  = new QLabel("Max");

    minLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    stepLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    gLayout->addWidget(minLabel, 0, 0);
    gLayout->addWidget(Min, 0, 1);
    gLayout->addWidget(maxLabel, 0, 2);
    gLayout->addWidget(Max, 0, 3);
    gLayout->addWidget(stepLabel, 0, 4);
    gLayout->addWidget(Step, 0, 5);

    for (auto* spinBox : std::vector({ Current, Min, Max })) {
        spinBox->setValue(property.Get());
        spinBox->setRange(property.GetRange().Min, property.GetRange().Max);
        spinBox->setSingleStep(property.GetRange().Step);
        spinBox->setDecimals(property.GetRange().Decimals);
    }

    Step->setRange(0, 2000.0);
    Step->setSingleStep(property.GetRange().Step);
    Step->setDecimals(property.GetRange().Decimals);

    auto currentValue = property.Get();
    Current->setValue(currentValue);
    Min->setValue(currentValue);
    Max->setValue(currentValue);

    fLayout->addRow("Range", rangeWidget);

    for (auto* spinBox : std::vector({ Min, Max, Step }))
        connect(spinBox, &QDoubleSpinBox::valueChanged,
                this, [this]() { Q_EMIT ValueChanged(); });
}

auto FloatParameterSpanWidget::SetParameterSpan(ParameterSpan<float> const& floatParameterSpan) noexcept -> void {
    auto numbers = floatParameterSpan.GetNumbers();
    Min->setValue(numbers.Min);
    Max->setValue(numbers.Max);
    Step->setValue(numbers.Step);

    for (auto* spinBox : std::vector{ Min, Max, Step })
        spinBox->setEnabled(false);
}

auto FloatParameterSpanWidget::GetParameterSpanData() -> ParameterSpanData {
    return { Property, { static_cast<float>(Min->value()),
                         static_cast<float>(Max->value()),
                         static_cast<float>(Step->value()) } };
}


FloatPointParameterSpanWidget::FloatPointParameterSpanWidget(FloatPointObjectProperty const& property,
                                                             QWidget* parent) :
        QWidget(parent),
        Property(property),
        Current (new DoubleCoordinateRowWidget({ property.GetRange().Min, property.GetRange().Max, property.GetRange().Step,
                                           0.0, property.GetRange().Decimals })),
        MinMaxStep([&]() {
            auto* coordinateRowWidget = new DoubleCoordinateRowWidget(true);
            auto range = property.GetRange();
            coordinateRowWidget->AppendCoordinatesRow({ range.Min, range.Max, range.Step, 0.0, range.Decimals }, "Min");
            coordinateRowWidget->AppendCoordinatesRow({ range.Min, range.Max, range.Step, 0.0, range.Decimals }, "Max");
            coordinateRowWidget->AppendCoordinatesRow({ std::max(range.Min, 0.0F), range.Max, range.Step,
                                                        0.0, range.Decimals }, "Step");
            return coordinateRowWidget;
        }()) {

    auto* fLayout = new QFormLayout(this);

    fLayout->addRow("Current", Current);
    auto currentData = DoubleCoordinateRowWidget::RowData(property.Get());
    Current->SetRowData(0, currentData);
    Current->setEnabled(false);

    fLayout->addRow(MinMaxStep);
    MinMaxStep->SetRowData(0, currentData);
    MinMaxStep->SetRowData(1, currentData);

    connect(MinMaxStep, &DoubleCoordinateRowWidget::ValueChanged,
            this, [this]() { Q_EMIT ValueChanged(); });
}

auto FloatPointParameterSpanWidget::SetParameterSpan(ParameterSpan<FloatPoint> const& parameterSpan) noexcept -> void {
    auto numbers = parameterSpan.GetNumbers();
    MinMaxStep->SetRowData(0, DoubleCoordinateRowWidget::RowData(numbers.Min));
    MinMaxStep->SetRowData(1, DoubleCoordinateRowWidget::RowData(numbers.Max));
    MinMaxStep->SetRowData(2, DoubleCoordinateRowWidget::RowData(numbers.Step));

    MinMaxStep->setEnabled(false);
}

auto FloatPointParameterSpanWidget::GetParameterSpanData() -> ParameterSpanData {
    FloatPoint const min = MinMaxStep->GetRowData(0).ToFloatArray();
    FloatPoint const max = MinMaxStep->GetRowData(1).ToFloatArray();
    FloatPoint const step = MinMaxStep->GetRowData(2).ToFloatArray();

    return { Property, { min, max, step } };
}


ObjectPropertyGroup::ObjectPropertyGroup(ArtifactVariantPointer artifactVariantPointer, QWidget* parent) :
        QGroupBox(parent),
        VLayout(new QVBoxLayout(this)),
        ParameterProperties(artifactVariantPointer.GetProperties()),
        SelectPropertyComboBox([&]() {
            auto* comboBox = new QComboBox();

            auto propertyNames = ParameterProperties.GetNames();
            for (auto const& name : propertyNames)
                comboBox->addItem(QString::fromStdString(name));

            return comboBox;
        }()) {

    setTitle("Property");

    VLayout->addWidget(SelectPropertyComboBox);

    UpdatePropertyWidget();

    connect(SelectPropertyComboBox, &QComboBox::currentIndexChanged,
            this, [this]() { UpdatePropertyWidget(); });
}

auto ObjectPropertyGroup::GetParameterSpan(ArtifactVariantPointer artifactPointer,
                                           std::string name) -> PipelineParameterSpan {
    return std::visit(Overload {
        [&, artifactPointer](FloatParameterSpanWidget* widget) -> PipelineParameterSpan {
            auto data = widget->GetParameterSpanData();
            return ParameterSpan<float> { artifactPointer, data.Property, data.Numbers, name };
        },
        [&, artifactPointer](FloatPointParameterSpanWidget* widget) -> PipelineParameterSpan {
            auto data = widget->GetParameterSpanData();
            return ParameterSpan<FloatPoint> { artifactPointer, data.Property, data.Numbers, name };
        }
    }, PropertyWidget);
}

auto ObjectPropertyGroup::UpdatePropertyWidget() -> void {
    int const idx = SelectPropertyComboBox->currentIndex();

    if (idx == -1)
        return;

    std::visit([this](auto* widget) {
        if (!widget)
            return;

        VLayout->removeWidget(widget);

        delete widget;
    }, PropertyWidget);

    auto& property = ParameterProperties.At(idx);
    PropertyWidget = std::visit(Overload {
        [](FloatObjectProperty& p) -> PropertyWidgetPointer { return new FloatParameterSpanWidget(p); },
        [](FloatPointObjectProperty& p) -> PropertyWidgetPointer { return new FloatPointParameterSpanWidget(p); },
    }, property.Variant());

    std::visit([this](auto* widget) { VLayout->addWidget(widget); }, PropertyWidget);

    std::visit(Overload {
        [this](FloatParameterSpanWidget* widget) -> void  {
            connect(widget, &FloatParameterSpanWidget::ValueChanged, this, &ObjectPropertyGroup::ValueChanged);
        },
        [this](FloatPointParameterSpanWidget* widget) -> void {
            connect(widget, &FloatPointParameterSpanWidget::ValueChanged, this, &ObjectPropertyGroup::ValueChanged);
        },
    }, PropertyWidget);
}

auto ObjectPropertyGroup::SetParameterSpan(PipelineParameterSpan const& parameterSpan) -> void {
    int const propertyIdx = SelectPropertyComboBox->findText(QString::fromStdString(parameterSpan.GetPropertyName()));
    if (propertyIdx == -1)
        throw std::runtime_error("Property not found");

    SelectPropertyComboBox->setCurrentIndex(propertyIdx);
    UpdatePropertyWidget();

    SelectPropertyComboBox->setEnabled(false);

    std::visit(Overload {
        [&](FloatParameterSpanWidget* widget) {
            widget->SetParameterSpan(std::get<ParameterSpan<float>>(parameterSpan.SpanVariant));
        },
        [&](FloatPointParameterSpanWidget* widget) {
            widget->SetParameterSpan(std::get<ParameterSpan<FloatPoint>>(parameterSpan.SpanVariant));
        }
    }, PropertyWidget);
}
