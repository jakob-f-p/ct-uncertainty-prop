#pragma once

#include "SimpleTransform.h"
#include "../Concepts.h"
#include "../Enum.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QWidget>

using StructureId = int32_t;
using uidx_t = uint16_t;
using idx_t = int32_t;

namespace CtStructure {
    auto
    AddCoordinatesRow(const QString& baseName, const QString& labelText,
                      double minValue, double maxValue, double stepSize,
                      QGridLayout* gridLayout, int gridLayoutRow,
                      double defaultValue) noexcept -> void;

    [[nodiscard]] auto
    GetCoordinatesRow(const QString& baseName,
                      double minValue, double maxValue, double stepSize) noexcept -> QWidget*;

    [[nodiscard]] auto
    GetAxisSpinBoxName(const QString& transformName, const QString& axisName) noexcept -> QString;

    const QStringList AxisNames = { "x", "y", "z" };

    const QStringList TransformNames { "Translate", "Rotate", "Scale" };
}


template<typename T>
concept CtStructureLike = requires(T structure,
                                   Point position,
                                   SimpleTransformData transformData,
                                   idx_t structureIdx,
                                   T::Data data) {
    typename T::Data;

    structure.SetTransformData(transformData);
    { structure.GetTransformData() } -> std::same_as<SimpleTransformData>;

    structure.SetParentIdx(structureIdx);
    { structure.GetParentIdx() } -> std::same_as<idx_t>;

    structure.IncrementParentIdx();
    structure.DecrementParentIdx();

    { structure.GetData() } -> std::same_as<typename T::Data>;

    structure.SetData(data);
};

template<typename T>
concept TCtStructure = CtStructureLike<T>
                        && HasMTime<T>
                        && IsNamed<T>;

class QFormLayout;
class QWidget;

template<typename T>
concept TStructureData = requires(T derivedData,
                                  T::Structure structure,
                                  QFormLayout* fLayout,
                                  QWidget* widget) {
    derivedData.PopulateDerivedStructure(structure);
    derivedData.PopulateFromDerivedStructure(structure);

    T::AddSubTypeWidgets(fLayout);
    derivedData.PopulateStructureWidget(widget);
    derivedData.PopulateFromStructureWidget(widget);
};



class CtStructureBase {
    Q_GADGET
public:
    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return Transform.GetMTime(); };

    auto
    Modified() noexcept -> void { return Transform.Modified(); };

    auto
    SetName(std::string&& name) noexcept -> void { Name = std::move(name); };

    [[nodiscard]] auto
    GetName() const -> std::string { return Name; };

    [[nodiscard]] auto
    GetTransformedPoint(Point point) const noexcept -> Point { return Transform.TransformPoint(point); }

    [[nodiscard]] auto
    GetTransformData() const noexcept -> SimpleTransformData { return Transform.GetData(); };

    auto
    SetTransformData(const SimpleTransformData& transformData) noexcept -> void { Transform.SetData(transformData); }

    auto
    SetParentIdx(idx_t parentId) noexcept -> void { ParentIdx = parentId; }

    [[nodiscard]] auto
    GetParentIdx() const noexcept -> idx_t { return ParentIdx; }

    auto
    IncrementParentIdx() noexcept -> uidx_t { return ++ParentIdx; };

    auto
    DecrementParentIdx() noexcept -> uidx_t { return --ParentIdx; };

    struct ModelingResult {
        float FunctionValue;
        float Radiodensity;
        StructureId BasicCtStructureId;
    };

    enum struct FunctionType : uint8_t {
        SPHERE,
        BOX,
        CONE
    };
    Q_ENUM(FunctionType);

    [[nodiscard]] auto static
    FunctionTypeToString(FunctionType functionType) noexcept -> std::string;
    ENUM_GET_VALUES(FunctionType, false)

    struct TissueType {
        std::string Name;
        float CtNumber = 0.0; // value on the Hounsfield scale

        auto
        operator==(const TissueType& other) const noexcept -> bool { return Name == other.Name && CtNumber == other.CtNumber; }

        friend auto operator<<(std::ostream& stream, const TissueType& type) noexcept -> std::ostream&;
    };
    static std::map<std::string, TissueType> TissueTypeMap;

    [[nodiscard]] auto static
    GetTissueTypeByName(const std::string& tissueName) noexcept -> TissueType;

    [[nodiscard]] auto static
    GetTissueTypeNames() noexcept -> QStringList;

    enum struct OperatorType : uint8_t {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        INVALID
    };
    Q_ENUM(OperatorType);

    [[nodiscard]] auto static
    OperatorTypeToString(OperatorType operatorType) noexcept-> std::string;
    ENUM_GET_VALUES(OperatorType, true);

protected:
    template<TStructureData StructureData> friend class CtStructureBaseData;

    CtStructureBase() = default;

    SimpleTransform Transform;
    std::string Name;

    idx_t ParentIdx = -1;

    static std::atomic<StructureId> GlobalBasicStructureId;
};



template<TStructureData StructureData>
class CtStructureBaseData {
    using Structure = StructureData::Structure;

public:
    QString Name;
    QString ViewName;
    SimpleTransformData Transform = {};
    StructureData Data;

    auto
    PopulateStructure(Structure& structure) const -> void;

    auto
    PopulateFromStructure(const Structure& structure) -> void;

    [[nodiscard]] static auto
    GetWidget() noexcept -> QWidget*;

    auto
    PopulateWidget(QWidget* widget) const -> void;

    auto
    PopulateFromWidget(QWidget* widget) -> void;

private:
    static const QString NameEditObjectName;
};

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::PopulateStructure(Structure& structure) const -> void {
    structure.SetName(Name.toStdString());
    structure.SetTransformData(Transform);

    Data.PopulateDerivedStructure(structure);
}

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::PopulateFromStructure(const Structure& structure) -> void {
    Name = QString::fromStdString(structure.Name);
    ViewName = QString::fromStdString(structure.GetViewName());
    Transform = structure.GetTransformData();

    Data.PopulateFromDerivedStructure(structure);
}

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::GetWidget() noexcept -> QWidget* {
    auto* widget = new QWidget();
    auto* fLayout = new QFormLayout(widget);
    fLayout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    fLayout->setHorizontalSpacing(15);

    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName(NameEditObjectName);
    fLayout->addRow("Name", nameLineEdit);

    StructureData::AddSubTypeWidgets(fLayout);

    auto* transformGroup = new QGroupBox("Transform");
    auto* transformGLayout = new QGridLayout(transformGroup);
    transformGLayout->setColumnStretch(0, 1);
    std::array<std::array<double, 2>, 3> transformRanges { -100.0, 100.0, 0.0, 360.0, -10.0, 10.0 };
    std::array<double, 3> transformStepSizes { 2.0, 1.0, 0.1 };
    for (int i = 0; i < CtStructure::TransformNames.size(); i++) {
        CtStructure::AddCoordinatesRow(CtStructure::TransformNames[i], CtStructure::TransformNames[i],
                                       transformRanges[i][0], transformRanges[i][1], transformStepSizes[i],
                                       transformGLayout, i, i == 2 ? 1.0 : 0.0);
    }
    fLayout->addRow(transformGroup);

    return widget;
}

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::PopulateWidget(QWidget* widget) const -> void {
    if (!widget)
        throw std::runtime_error("Given widget was nullptr");

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    nameLineEdit->setText(Name);

    for (int i = 0; i < Transform.size(); ++i) {
        for (int j = 0; j < Transform[0].size(); ++j) {
            auto* spinBox = widget->findChild<QDoubleSpinBox*>(
                    CtStructure::GetAxisSpinBoxName(CtStructure::TransformNames[i], CtStructure::AxisNames[j]));
            spinBox->setValue(Transform[i][j]);
        }
    }

    Data.PopulateStructureWidget(widget);
}

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::PopulateFromWidget(QWidget* widget) -> void {
    if (!widget)
        throw std::runtime_error("Given widget was nullptr");

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    Name = nameLineEdit->text();

    for (int i = 0; i < Transform.size(); ++i) {
        for (int j = 0; j < Transform[0].size(); ++j) {
            auto* spinBox = widget->findChild<QDoubleSpinBox*>(
                    CtStructure::GetAxisSpinBoxName(CtStructure::TransformNames[i], CtStructure::AxisNames[j]));
            Transform[i][j] = static_cast<float>(spinBox->value());
        }
    }

    Data.PopulateFromStructureWidget(widget);
}

template<TStructureData StructureData>
const QString CtStructureBaseData<StructureData>::NameEditObjectName = "NameEdit";
