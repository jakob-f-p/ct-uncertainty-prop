#pragma once

#include "CtStructure.h"
#include "../Artifacts/Types.h"

#include <vtkBox.h>
#include <vtkCone.h>
#include <vtkCylinder.h>
#include <vtkImplicitBoolean.h>
#include <vtkNew.h>
#include <vtkPlane.h>
#include <vtkSphere.h>

class QFormLayout;
class QString;
class QWidget;


template<typename T>
concept Evaluable = requires(T structure, T::Data data) {
    structure.AddFunctionData(data);
    structure.SetFunctionData(data);
    { structure.EvaluateFunction(Point{}) } -> std::same_as<float>;
};

template<typename T>
concept HasWidget = requires(T::Data data, T::Data::Widget* widget) {
    data.PopulateFromWidget(widget);
    data.PopulateWidget(widget);
};

template<typename T>
concept TBasicStructure = Evaluable<T>
                            && HasWidget<T>
                            && std::equality_comparable<T>;



class SphereWidget;

struct SphereData {
    double Radius = 1.0;
    Point Center = {};

    using Widget = SphereWidget;

    auto
    PopulateFromWidget(Widget* widget) noexcept -> void;

    auto
    PopulateWidget(Widget* widget) const noexcept -> void;
};


class SphereWidget : public QWidget {
    Q_OBJECT

public:
    SphereWidget();

    [[nodiscard]] auto
    GetData() noexcept -> SphereData;

    auto
    Populate(const SphereData& data) noexcept -> void;

private:
    QDoubleSpinBox* RadiusSpinBox;
    DoubleCoordinateRowWidget* CenterCoordinateRow;
};



struct Sphere {
    using Data = SphereData;

    Sphere() { Function->SetRadius(10.0); }

    auto
    AddFunctionData(Data& data) const noexcept -> void;

    auto
    SetFunctionData(const Data& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateFunction(Point point) const noexcept -> float {
        return static_cast<float>(Function->EvaluateFunction(point.data()));
    }

    auto
    operator==(const Sphere& other) const noexcept -> bool { return Function == other.Function; }

private:
    vtkNew<vtkSphere> Function;
};

static_assert(TBasicStructure<Sphere>);



class BoxWidget;

struct BoxData {
    Point MinPoint {};
    Point MaxPoint {};

    using Widget = BoxWidget;

    auto
    PopulateFromWidget(Widget* widget) noexcept -> void;

    auto
    PopulateWidget(Widget* widget) const noexcept -> void;

private:
    static const QString BoxMinPointName;
    static const QString BoxMaxPointName;
};

struct Box {
    using Data = BoxData;

    Box() { Function->SetBounds(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0); }

    auto
    AddFunctionData(Data& data) const noexcept -> void;

    auto
    SetFunctionData(const Data& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateFunction(Point point) const noexcept -> float {
        return static_cast<float>(Function->EvaluateFunction(point.data()));
    }

    auto
    operator==(const Box& other) const noexcept -> bool { return Function == other.Function; }

private:
    vtkNew<vtkBox> Function;
};

class BoxWidget : public QWidget {
    Q_OBJECT

public:
    BoxWidget();

    [[nodiscard]] auto
    GetData() noexcept -> BoxData;

    auto
    Populate(const BoxData& data) noexcept -> void;

private:
    DoubleCoordinateRowWidget* MinMaxPointWidget;
};

static_assert(TBasicStructure<Box>);



class ConeWidget;

struct ConeData {
    double Radius = 5.0;
    double Height = 10.0;

    using Widget = ConeWidget;

    auto
    PopulateFromWidget(Widget* widget) noexcept -> void;

    auto
    PopulateWidget(Widget* widget) const noexcept -> void;
};


class ConeWidget : public QWidget {
    Q_OBJECT

public:
    ConeWidget();

    [[nodiscard]] auto
    GetData() noexcept -> ConeData;

    auto
    Populate(ConeData const& data) noexcept -> void;

private:
    QDoubleSpinBox* RadiusSpinBox;
    QDoubleSpinBox* HeightSpinBox;
};


struct Cone {
    using Data = ConeData;

    Cone();

    auto
    AddFunctionData(Data& data) const noexcept -> void;

    auto
    SetFunctionData(Data const& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateFunction(Point point) const noexcept -> float {
        return static_cast<float>(ConeFunction->EvaluateFunction(point.data()));
    }

    [[nodiscard]] auto
    operator==(Cone const& other) const noexcept -> bool { return ConeFunction == other.ConeFunction; }
private:
    vtkNew<vtkCone> UnboundedCone;
    vtkNew<vtkPlane> TipPlane;
    vtkNew<vtkPlane> BasePlane;
    vtkNew<vtkImplicitBoolean> ConeFunction;
};

static_assert(TBasicStructure<Cone>);



class CylinderWidget;

struct CylinderData {
    double Radius = 5.0;
    double Height = 10.0;

    using Widget = CylinderWidget;

    auto
    PopulateFromWidget(Widget* widget) noexcept -> void;

    auto
    PopulateWidget(Widget* widget) const noexcept -> void;
};


class CylinderWidget : public QWidget {
    Q_OBJECT

public:
    CylinderWidget();

    [[nodiscard]] auto
    GetData() noexcept -> CylinderData;

    auto
    Populate(CylinderData const& data) noexcept -> void;

private:
    QDoubleSpinBox* RadiusSpinBox;
    QDoubleSpinBox* HeightSpinBox;
};


struct Cylinder {
    using Data = CylinderData;

    Cylinder();

    auto
    AddFunctionData(Data& data) const noexcept -> void;

    auto
    SetFunctionData(Data const& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateFunction(Point point) const noexcept -> float {
        return static_cast<float>(CylinderFunction->EvaluateFunction(point.data()));
    }

    [[nodiscard]] auto
    operator==(Cylinder const& other) const noexcept -> bool { return CylinderFunction == other.CylinderFunction; }
private:
    vtkNew<vtkCylinder> UnboundedCylinder;
    vtkNew<vtkPlane> BottomPlane;
    vtkNew<vtkPlane> TopPlane;
    vtkNew<vtkImplicitBoolean> CylinderFunction;
};

static_assert(TBasicStructure<Cylinder>);



#define SHAPE_TYPES Sphere, Box, Cone, Cylinder

using ShapeVariant = std::variant<SHAPE_TYPES>;
using ShapeDataVariant = DataVariant<SHAPE_TYPES>;
using ShapeWidgetVariant = WidgetPointerVariant<SHAPE_TYPES>;

#undef SHAPE_TYPES
