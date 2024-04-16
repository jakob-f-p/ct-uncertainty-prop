#pragma once

#include "BasicStructures.h"
#include "CtStructure.h"


class QFormLayout;
class QGroupBox;

template<class... Ts>
struct Overload : Ts... { using Ts::operator()...; };


template<TBasicStructure StructureImpl> class BasicStructureBase;

class BasicStructureBaseUi {
protected:
    friend class BasicStructureUi;

    static const QString FunctionTypeComboBoxName;
    static const QString TissueTypeComboBoxName;
    static const QString FunctionParametersGroupName;
};

template<TBasicStructure Impl>
struct BasicStructureBaseData : BasicStructureBaseUi {
    using Structure = BasicStructureBase<Impl>;

    CtStructureBase::FunctionType FunctionType = Impl::GetFunctionType();
    QString TissueName;
    Impl::Data Data;

    auto
    PopulateDerivedStructure(Structure& structure) const noexcept -> void;

    auto
    PopulateFromDerivedStructure(const Structure& structure) noexcept -> void;

    static auto
    AddSubTypeWidgets(QFormLayout* fLayout) -> void;

    auto
    PopulateStructureWidget(QWidget* widget) const -> void;

    auto
    PopulateFromStructureWidget(QWidget* widget) -> void;

private:
    [[nodiscard]] static auto
    GetFunctionParametersGroup(CtStructureBase::FunctionType functionType) -> QGroupBox*;

    static auto
    UpdateFunctionParametersGroup(QFormLayout* fLayout) -> void;
};

using SphereData = CtStructureBaseData<BasicStructureBaseData<SphereStructureImpl>>;
using BoxData = CtStructureBaseData<BasicStructureBaseData<BoxStructureImpl>>;
using BasicStructureDataVariant = std::variant<SphereData, BoxData>;


template<TBasicStructure Impl>
class BasicStructureBase : public CtStructureBase {
public:
    using Data = CtStructureBaseData<BasicStructureBaseData<Impl>>;

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetFunctionType() const noexcept -> FunctionType;

    auto
    SetTissueType(TissueType tissueType) noexcept -> void;

    [[nodiscard]] inline auto
    FunctionValue(Point point) const noexcept -> float;

    auto
    GetData() const noexcept -> Data;

    auto
    SetData(const Data& data) noexcept -> void;

    auto
    operator==(const BasicStructureBase& other) const noexcept -> bool { return Id == other.Id && Tissue == other.Tissue; }

private:
    using Base = CtStructureBase;

    friend struct EvaluateImplicitStructures;
    template<TBasicStructure BasicStructureImpl> friend class BasicStructureBaseData;

    StructureId Id = ++GlobalBasicStructureId;
    TissueType Tissue = GetTissueTypeByName("Air");
    Impl BasicStructureImpl {};
};

template<TBasicStructure BasicStructureImpl>
auto BasicStructureBase<BasicStructureImpl>::FunctionValue(Point point) const noexcept -> float {
    Point transformedPoint = Transform.TransformPoint(point);

    return BasicStructureImpl.EvaluateFunction(transformedPoint);
}

using SphereStructure = BasicStructureBase<SphereStructureImpl>;
using BoxStructure = BasicStructureBase<BoxStructureImpl>;
using BasicStructureVariant = std::variant<SphereStructure, BoxStructure>;



namespace BasicStructure {
    [[nodiscard]] auto
    CreateBasicStructure(const BasicStructureDataVariant& dataVariant) -> BasicStructureVariant;
}

class BasicStructureUi {
public:
    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> BasicStructureDataVariant;

    [[nodiscard]] auto static
    GetWidget() -> QWidget*;
};
