#pragma once

#include "../../PipelineGroups/ArtifactVariantPointer.h"
#include "../../PipelineGroups/ObjectProperty.h"
#include "../../PipelineGroups/PipelineParameterSpan.h"

#include <QWidget>
#include <QGroupBox>

class CoordinateRowWidget;
class NameLineEdit;

class QComboBox;
class QDoubleSpinBox;
class QVBoxLayout;

class FloatParameterSpanWidget : public QWidget {
public:
    explicit FloatParameterSpanWidget(FloatObjectProperty const& property, QWidget* parent = nullptr);

    auto
    SetParameterSpan(ParameterSpan<float> floatParameterSpan) noexcept -> void;

private:
    NameLineEdit* NameEdit;

    QDoubleSpinBox* Current;

    QDoubleSpinBox* Min;
    QDoubleSpinBox* Step;
    QDoubleSpinBox* Max;
};


class FloatPointParameterSpanWidget : public QWidget {
public:
    explicit FloatPointParameterSpanWidget(FloatPointObjectProperty const& property, QWidget* parent = nullptr);

    auto
    SetParameterSpan(ParameterSpan<FloatPoint> floatParameterSpan) noexcept -> void;

private:
    NameLineEdit* NameEdit;

    CoordinateRowWidget* Current;

    CoordinateRowWidget* Min;
    CoordinateRowWidget* Step;
    CoordinateRowWidget* Max;
};


class ObjectPropertyGroup : public QGroupBox {
    Q_OBJECT

public:
    explicit ObjectPropertyGroup(ArtifactVariantPointer artifactVariantPointer, QWidget* parent = nullptr);

private:
    auto
    UpdatePropertyWidget();

    using PropertyWidgetPointer = std::variant<FloatParameterSpanWidget*, FloatPointParameterSpanWidget*>;

    QVBoxLayout* VLayout;
    PipelineParameterProperties ParameterProperties;
    QComboBox* SelectPropertyComboBox;
    PropertyWidgetPointer PropertyWidget;
};