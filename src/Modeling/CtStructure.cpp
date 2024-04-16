#include "CtStructure.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QWidget>

auto CtStructureBase::FunctionTypeToString(FunctionType functionType) noexcept -> std::string {
    switch (functionType) {
        case FunctionType::SPHERE: return "Sphere";
        case FunctionType::BOX:    return "Box";
        case FunctionType::CONE:   return "Cone";
        default: { qWarning("No matching implicit function type found"); return ""; }
    }
}

auto CtStructureBase::OperatorTypeToString(OperatorType operatorType) noexcept-> std::string {
    switch (operatorType) {
        case OperatorType::UNION:        return "Union";
        case OperatorType::INTERSECTION: return "Intersection";
        case OperatorType::DIFFERENCE:   return "Difference";
        default: { qWarning("No string representation for this operator exists"); return ""; }
    }
}

auto CtStructureBase::GetTissueTypeByName(const std::string& tissueName) noexcept -> TissueType {
    if (auto search = TissueTypeMap.find(tissueName);
            search != TissueTypeMap.end())
        return search->second;

    qWarning("No tissue type with requested name present. Returning 'Air'");

    return TissueTypeMap.at("Air");
}

auto CtStructureBase::GetTissueTypeNames() noexcept -> QStringList {
    QStringList names;
    std::transform(TissueTypeMap.cbegin(), TissueTypeMap.cend(), std::back_inserter(names),
                   [](const auto& type) { return QString::fromStdString(type.first); });
    return names;
}

std::map<std::string, CtStructureBase::TissueType> CtStructureBase::TissueTypeMap = {
        { "Air",             { "Air",            -1000.0f } },
        { "Fat",             { "Fat",             -100.0f } },
        { "Water",           { "Water",              0.0f } },
        { "Soft Tissue",     { "Soft Tissue",      200.0f } },
        { "Cancellous Bone", { "Cancellous Bone",  350.0f } },
        { "Cortical Bone",   { "Cortical Bone",    800.0f } },
        { "Metal",           { "Metal",          15000.0f } }
};

std::atomic<StructureId> CtStructureBase::GlobalBasicStructureId = 0;

auto CtStructure::AddCoordinatesRow(const QString& baseName, const QString& labelText,
                                    double minValue, double maxValue, double stepSize,
                                    QGridLayout* gridLayout, int gridLayoutRow,
                                    double defaultValue) noexcept -> void {
    auto* titleLabel = new QLabel(labelText);
    gridLayout->addWidget(titleLabel, gridLayoutRow, 0);

    for (int i = 0; i < AxisNames.size(); i++) {
        auto* coordinateLabel = new QLabel(AxisNames[i]);
        coordinateLabel->setMinimumWidth(15);
        coordinateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gridLayout->addWidget(coordinateLabel, gridLayoutRow, 1 + 2*i);
        auto* coordinateSpinBox = new QDoubleSpinBox();
        coordinateSpinBox->setObjectName(GetAxisSpinBoxName(baseName, AxisNames[i]));
        coordinateSpinBox->setRange(minValue, maxValue);
        coordinateSpinBox->setSingleStep(stepSize);
        coordinateSpinBox->setValue(defaultValue);
        gridLayout->addWidget(coordinateSpinBox, gridLayoutRow, 2 + 2*i);
    }
}

auto CtStructure::GetCoordinatesRow(const QString& baseName,
                                    double minValue, double maxValue, double stepSize) noexcept -> QWidget* {
    auto* widget = new QWidget();
    auto* hLayout = new QHBoxLayout(widget);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch();

    for (int i = 0; i < AxisNames.size(); i++) {
        auto* coordinateLabel = new QLabel(AxisNames[i]);
        coordinateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (i > 0)
            coordinateLabel->setMinimumWidth(15);
        hLayout->addWidget(coordinateLabel);

        auto* coordinateSpinBox = new QDoubleSpinBox();
        coordinateSpinBox->setObjectName(GetAxisSpinBoxName(baseName, AxisNames[i]));
        coordinateSpinBox->setRange(minValue, maxValue);
        coordinateSpinBox->setSingleStep(stepSize);
        hLayout->addWidget(coordinateSpinBox);
    }

    return widget;
}

auto CtStructure::GetAxisSpinBoxName(const QString& transformName, const QString& axisName) noexcept -> QString {
    return transformName + axisName;
}
