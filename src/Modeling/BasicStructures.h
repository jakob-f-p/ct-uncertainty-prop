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
#include <vtkMath.h>

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

struct PointDistancePair {
    DoublePoint Point;
    float Distance;
};



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

    [[nodiscard]] auto
    ClosestPointOnXYPlane(Point point) const -> std::optional<DoublePoint> {
        double const radius3d = Function->GetRadius();
        double const radius2dSquared = radius3d * radius3d - point[2] * point[2];
        if (radius2dSquared <= 0.0F)
            return std::nullopt;

        double const radius2d = sqrt(radius2dSquared);

        double const distanceCenter2d = sqrt(point[0] * point[0] + point[1] * point[1]);
        std::array<double, 2> const normalizedDifference2d { point[0] / distanceCenter2d, point[1] / distanceCenter2d };

        std::array<double, 2> const closestPoint2d { normalizedDifference2d[0] * radius2d,
                                                     normalizedDifference2d[1] * radius2d };

        return DoublePoint { closestPoint2d[0], closestPoint2d[1], point[2] };
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

    [[nodiscard]] auto
    ClosestPointOnXYPlane(Point point) const -> std::optional<DoublePoint> {
        double const* bounds = Function->GetBounds();
        if (point[2] < bounds[4] || point[2] > bounds[5])
            return std::nullopt;

        static constexpr auto compareDistances = [](float x) {
            return [&](float const& a, float const& b) { return std::abs(a - x) < std::abs(b - x); };
        };

        std::array<double, 2> const closestPoint2d { std::max(bounds[0], bounds[1], compareDistances(point[0])),
                                                     std::max(bounds[2], bounds[3], compareDistances(point[1])) };

        return DoublePoint { closestPoint2d[0], closestPoint2d[1], point[2] };
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
    ClosestPointOnXYPlane(Point point) const -> std::optional<DoublePoint> {
        double const angleRad = vtkMath::RadiansFromDegrees(UnboundedCone->GetAngle());
        double const height = BasePlane->GetOrigin()[0];
        double const radius = height * tan(angleRad);
        if (point[2] > std::abs(radius))
            return std::nullopt;

        double const theta = vtkMath::RadiansFromDegrees(UnboundedCone->GetAngle());
        double const cosTheta = cos(theta);
        // approximation assuming d^2 = (x - x_0)^2 + (y - y_0)^2 = (x - x_0)^2 + y^2 + y_0^2
        // because not solvable in general
        double const nearestX = point[0] * cosTheta * cosTheta;

        // substitute into cone equation and solve
        double const tanTheta = tan(theta);
        double const nearestY = sqrt(point[0] * point[0] * tanTheta * tanTheta - point[2] * point[2]);

        return DoublePoint { nearestX, nearestY, point[2] };
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
    ClosestPointOnXYPlane(Point point) const -> std::optional<DoublePoint> {
        double const radius = UnboundedCylinder->GetRadius();
        if (point[2] > std::abs(radius))
            return std::nullopt;

        double const nearestX = sqrt(radius * radius - point[2] * point[2]);
        double const nearestY = point[1];

        return DoublePoint { nearestX, nearestY, point[2] };
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
