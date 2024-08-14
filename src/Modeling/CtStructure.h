#pragma once

#include "../Utils/IndexTypes.h"
#include "../Utils/SimpleTransform.h"

#include <vtkTransform.h>

class NameLineEdit;

class QFormLayout;
class QWidget;

template<typename T>
concept TStructureData = requires(T derivedData, T::Structure structure) {
    derivedData.PopulateStructure(structure);
    derivedData.PopulateFromStructure(structure);
};

template<typename T>
concept TStructureWidget = requires(T widget, T::Data data) {
    widget.GetData();
    widget.Populate(data);
};



class CtStructureBase {
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
    GetInverselyTransformedPoint(Point point) const noexcept -> Point { return Transform.InverseTransformPoint(point); }

    auto
    SetTransformData(SimpleTransformData const& transformData) noexcept -> void { Transform.SetData(transformData); }

    [[nodiscard]] auto
    GetTransformData() const noexcept -> SimpleTransformData { return Transform.GetData(); };

protected:
    CtStructureBase() = default;

private:
    template<TStructureData StructureData> friend class CtStructureBaseData;
    friend class CtStructureTree;
    friend class CtStructureTreeModel;

    SimpleTransform Transform;
    std::string Name;
    idx_t ParentIdx;
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
    PopulateFromStructure(Structure const& structure) -> void;
};

template<TStructureWidget StructureWidget, typename Data>
class CtStructureBaseWidget : public QWidget {
public:
    CtStructureBaseWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> Data;

    auto
    Populate(Data const& data) noexcept -> void;

private:
    QFormLayout* Layout;
    NameLineEdit* NameEdit;
    StructureWidget* SubWidget;
    SimpleTransformWidget* TransformWidget;
};
