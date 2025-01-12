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
        Step(new QDoubleSpinBox()),
        Max (new QDoubleSpinBox()) {

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

    Step->setRange(0, 3000.0);
    Step->setSingleStep(property.GetRange().Step);
    Step->setDecimals(property.GetRange().Decimals);

    auto const currentValue = property.Get();
    Current->setValue(currentValue);
    Min->setValue(currentValue);
    Max->setValue(currentValue);

    fLayout->addRow("Range", rangeWidget);

    for (auto const* spinBox : std::vector({ Min, Max, Step }))
        connect(spinBox, &QDoubleSpinBox::valueChanged,
                this, [this] { Q_EMIT ValueChanged(); });
}

auto FloatParameterSpanWidget::SetParameterSpan(ParameterSpan<float> const& floatParameterSpan) const noexcept -> void {
    auto [min, max, step] = floatParameterSpan.GetNumbers();
    Min->setValue(min);
    Max->setValue(max);
    Step->setValue(step);

    for (auto* spinBox : std::vector{ Min, Max, Step })
        spinBox->setEnabled(false);
}

auto FloatParameterSpanWidget::GetParameterSpanData() const -> ParameterSpanData {
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
        MinMaxStep([&] {
            auto* coordinateRowWidget = new DoubleCoordinateRowWidget(true);
            auto [min, max, step, decimals] = property.GetRange();
            coordinateRowWidget->AppendCoordinatesRow({ min, max, step, 0.0, decimals }, "Min");
            coordinateRowWidget->AppendCoordinatesRow({ min, max, step, 0.0, decimals }, "Max");
            coordinateRowWidget->AppendCoordinatesRow({ std::max(min, 0.0F), max, step,
                                                        0.0, decimals }, "Step");
            return coordinateRowWidget;
        }()) {

    auto* fLayout = new QFormLayout(this);

    fLayout->addRow("Current", Current);
    auto const currentData = DoubleCoordinateRowWidget::RowData(property.Get());
    Current->SetRowData(0, currentData);
    Current->setEnabled(false);

    fLayout->addRow(MinMaxStep);
    MinMaxStep->SetRowData(0, currentData);
    MinMaxStep->SetRowData(1, currentData);

    connect(MinMaxStep, &DoubleCoordinateRowWidget::ValueChanged,
            this, [this] { Q_EMIT ValueChanged(); });
}

auto FloatPointParameterSpanWidget::SetParameterSpan(ParameterSpan<FloatPoint> const& parameterSpan) const noexcept -> void {
    auto const [min, max, step] = parameterSpan.GetNumbers();
    MinMaxStep->SetRowData(0, DoubleCoordinateRowWidget::RowData(min));
    MinMaxStep->SetRowData(1, DoubleCoordinateRowWidget::RowData(max));
    MinMaxStep->SetRowData(2, DoubleCoordinateRowWidget::RowData(step));

    MinMaxStep->setEnabled(false);
}

auto FloatPointParameterSpanWidget::GetParameterSpanData() const -> ParameterSpanData {
    FloatPoint const min = MinMaxStep->GetRowData(0).ToFloatArray();
    FloatPoint const max = MinMaxStep->GetRowData(1).ToFloatArray();
    FloatPoint const step = MinMaxStep->GetRowData(2).ToFloatArray();

    return { Property, { min, max, step } };
}


ObjectPropertyGroup::ObjectPropertyGroup(ArtifactVariantPointer artifactVariantPointer, QWidget* parent) :
        QGroupBox(parent),
        VLayout(new QVBoxLayout(this)),
        ParameterProperties(artifactVariantPointer.GetProperties()),
        SelectPropertyComboBox([&] {
            auto* comboBox = new QComboBox();

            for (auto const propertyNames = ParameterProperties.GetNames(); auto const& name : propertyNames)
                comboBox->addItem(QString::fromStdString(name));

            return comboBox;
        }()) {

    setTitle("Property");

    VLayout->addWidget(SelectPropertyComboBox);

    UpdatePropertyWidget();

    connect(SelectPropertyComboBox, &QComboBox::currentIndexChanged,
            this, [this] { UpdatePropertyWidget(); });
}

auto ObjectPropertyGroup::GetParameterSpan(ArtifactVariantPointer artifactPointer,
                                           std::string const& name) -> PipelineParameterSpan {
    return std::visit(Overload {
        [&, artifactPointer](FloatParameterSpanWidget const* widget) -> PipelineParameterSpan {
            auto const [property, numbers] = widget->GetParameterSpanData();
            return ParameterSpan { artifactPointer, property, numbers, name };
        },
        [&, artifactPointer](FloatPointParameterSpanWidget const* widget) -> PipelineParameterSpan {
            auto const [property, numbers] = widget->GetParameterSpanData();
            return ParameterSpan { artifactPointer, property, numbers, name };
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
        [](FloatObjectProperty const& p) -> PropertyWidgetPointer { return new FloatParameterSpanWidget(p); },
        [](FloatPointObjectProperty const& p) -> PropertyWidgetPointer { return new FloatPointParameterSpanWidget(p); },
    }, property.Variant());

    std::visit([this](auto* widget) { VLayout->addWidget(widget); }, PropertyWidget);

    std::visit(Overload {
        [this](FloatParameterSpanWidget const* widget) -> void  {
            connect(widget, &FloatParameterSpanWidget::ValueChanged, this, &ObjectPropertyGroup::ValueChanged);
        },
        [this](FloatPointParameterSpanWidget const* widget) -> void {
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
        [&](FloatParameterSpanWidget const* widget) {
            widget->SetParameterSpan(std::get<ParameterSpan<float>>(parameterSpan.SpanVariant));
        },
        [&](FloatPointParameterSpanWidget const* widget) {
            widget->SetParameterSpan(std::get<ParameterSpan<FloatPoint>>(parameterSpan.SpanVariant));
        }
    }, PropertyWidget);
}
