#pragma once

#include "../../PipelineGroups/ArtifactVariantPointer.h"
#include "../../PipelineGroups/ObjectProperty.h"
#include "../../PipelineGroups/PipelineParameterSpan.h"

#include <QWidget>
#include <QGroupBox>

class DoubleCoordinateRowWidget;
class NameLineEdit;

class QComboBox;
class QDoubleSpinBox;
class QVBoxLayout;

class FloatParameterSpanWidget : public QWidget {
    Q_OBJECT

public:
    explicit FloatParameterSpanWidget(FloatObjectProperty const& property, QWidget* parent = nullptr);

    auto
    SetParameterSpan(ParameterSpan<float> const& floatParameterSpan) noexcept -> void;

    struct ParameterSpanData {
        FloatObjectProperty Property;
        ParameterSpan<float>::NumberDetails Numbers;
    };

    [[nodiscard]] auto
    GetParameterSpanData() -> ParameterSpanData;

Q_SIGNALS:
    void ValueChanged();

private:
    FloatObjectProperty const& Property;

    QDoubleSpinBox* Current;
    QDoubleSpinBox* Min;
    QDoubleSpinBox* Step;
    QDoubleSpinBox* Max;
};


class FloatPointParameterSpanWidget : public QWidget {
    Q_OBJECT

public:
    explicit FloatPointParameterSpanWidget(FloatPointObjectProperty const& property, QWidget* parent = nullptr);

    auto
    SetParameterSpan(ParameterSpan<FloatPoint> const& parameterSpan) noexcept -> void;

    struct ParameterSpanData {
        FloatPointObjectProperty Property;
        ParameterSpan<FloatPoint>::NumberDetails Numbers;
    };

    [[nodiscard]] auto
    GetParameterSpanData() -> ParameterSpanData;

Q_SIGNALS:
    void ValueChanged();

private:
    FloatPointObjectProperty const& Property;

    DoubleCoordinateRowWidget* Current;
    DoubleCoordinateRowWidget* MinMaxStep;
};


class ObjectPropertyGroup : public QGroupBox {
    Q_OBJECT

public:
    explicit ObjectPropertyGroup(ArtifactVariantPointer artifactVariantPointer, QWidget* parent = nullptr);

    [[nodiscard]] auto
    GetParameterSpan(ArtifactVariantPointer artifactPointer, std::string name) -> PipelineParameterSpan;

    auto
    SetParameterSpan(PipelineParameterSpan const& parameterSpan) -> void;

Q_SIGNALS:
    void ValueChanged();

private:
    auto
    UpdatePropertyWidget() -> void;

    using PropertyWidgetPointer = std::variant<FloatParameterSpanWidget*, FloatPointParameterSpanWidget*>;

    QVBoxLayout* VLayout;
    PipelineParameterProperties ParameterProperties;
    QComboBox* SelectPropertyComboBox;
    PropertyWidgetPointer PropertyWidget;
};