#pragma once

#include "CtStructure.h"

#include <vtkBox.h>
#include <vtkNew.h>
#include <vtkSphere.h>

class QFormLayout;
class QString;
class QWidget;

template<typename T>
concept TBasicStructureImplLike = requires(T structure, T::Data data, QWidget* widget, QFormLayout* layout, Point point) {
    typename T::Data;

    structure.AddFunctionData(data);
    structure.SetFunctionData(data);
    { structure.EvaluateFunction(point) } -> std::same_as<float>;

    { T::GetFunctionType() } -> std::same_as<CtStructureBase::FunctionType>;

    T::Data::AddFunctionWidget(layout);

    data.PopulateFromWidget(widget);
    data.PopulateWidget(widget);
};

template<typename T>
concept TBasicStructure = TBasicStructureImplLike<T> && std::equality_comparable<T>;



struct SphereStructureImpl;


struct SphereDataImpl {
    double Radius = 1.0;
    Point Center = {};

    static auto
    AddFunctionWidget(QFormLayout* fLayout) noexcept -> void;

    auto
    PopulateFromWidget(QWidget* widget) noexcept -> void;

    auto
    PopulateWidget(QWidget* widget) const noexcept -> void;

private:
    static const QString SphereRadiusSpinBoxName;
    static const QString SphereCenterName;
};

struct SphereStructureImpl {
    using Data = SphereDataImpl;

    [[nodiscard]] constexpr static auto
    GetFunctionType() -> CtStructureBase::FunctionType { return CtStructureBase::FunctionType::SPHERE; };

    auto
    AddFunctionData(Data& data) const noexcept -> void;

    auto
    SetFunctionData(const Data& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateFunction(Point point) const noexcept -> float {
        return static_cast<float>(Function->EvaluateFunction( point.data()));
    }

    auto
    operator==(const SphereStructureImpl& other) const noexcept -> bool { return Function == other.Function; }

private:
    vtkNew<vtkSphere> Function;
};

static_assert(TBasicStructure<SphereStructureImpl>);




struct BoxDataImpl {
    Point MinPoint {};
    Point MaxPoint {};

    static auto
    AddFunctionWidget(QFormLayout* fLayout) noexcept -> void;

    auto
    PopulateFromWidget(QWidget* widget) noexcept -> void;

    auto
    PopulateWidget(QWidget* widget) const noexcept -> void;

private:
    static const QString BoxMinPointName;
    static const QString BoxMaxPointName;
};

struct BoxStructureImpl {
    using Data = BoxDataImpl;

    BoxStructureImpl() { Function->SetBounds(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0); }

    [[nodiscard]] constexpr static auto
    GetFunctionType() -> CtStructureBase::FunctionType { return CtStructureBase::FunctionType::BOX; };

    auto
    AddFunctionData(Data& data) const noexcept -> void;

    auto
    SetFunctionData(const Data& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateFunction(Point point) const noexcept -> float {
        return static_cast<float>(Function->EvaluateFunction(point.data()));
    }

    auto
    operator==(const BoxStructureImpl& other) const noexcept -> bool { return Function == other.Function; }

private:
    vtkNew<vtkBox> Function {};
};

static_assert(TBasicStructure<BoxStructureImpl>);
