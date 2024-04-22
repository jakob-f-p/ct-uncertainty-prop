#pragma once

#include "SimpleTransform.h"
#include "../Concepts.h"
#include "../Enum.h"

#include <vtkTransform.h>

class NameLineEdit;

class QFormLayout;
class QWidget;

using StructureId = int32_t;
using uidx_t = uint16_t;
using idx_t = int32_t;

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

template<typename T>
concept TStructureData = requires(T derivedData, T::Structure structure) {
    derivedData.PopulateStructure(structure);
    derivedData.PopulateFromStructure(structure);
};

template<typename T>
concept TStructureWidget = requires(T widget, T::Data data) {
    widget.AddData(data);
    widget.Populate(data);
};



class CtStructureBase {
    Q_GADGET

public:
    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return Transform.GetMTime(); };

    auto
    Modified() noexcept -> void { Transform.Modified(); };

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

protected:
    template<TStructureData StructureData> friend class CtStructureBaseData;

    CtStructureBase() = default;

    SimpleTransform Transform;
    std::string Name;

private:
    idx_t ParentIdx = -1;
};



template<TStructureData StructureData>
class CtStructureBaseData {
    using Structure = StructureData::Structure;

public:
    QString Name;
    QString ViewName;
    SimpleTransformData Transform {};
    StructureData Data;

    auto
    PopulateStructure(Structure& structure) const -> void;

    auto
    PopulateFromStructure(const Structure& structure) -> void;
};

template<TStructureWidget StructureWidget, typename Data>
class CtStructureBaseWidget : public QWidget {
public:
    CtStructureBaseWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> Data;

    auto
    Populate(const Data& data) noexcept -> void;

private:
    QFormLayout* Layout;
    NameLineEdit* NameEdit;
    StructureWidget* SubWidget;
    SimpleTransformWidget* TransformWidget;
};
