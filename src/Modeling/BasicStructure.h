#pragma once

#include "BasicStructures.h"
#include "CtStructure.h"

#include "../Utils/Enum.h"

class BasicStructure;

class QComboBox;
class QFormLayout;
class QGroupBox;

namespace BasicStructureDetails {
    Q_NAMESPACE

    enum struct FunctionType : uint8_t {
        SPHERE,
        BOX,
        CONE,
        CYLINDER
    };
    Q_ENUM_NS(FunctionType);

    [[nodiscard]] auto static
    FunctionTypeToString(FunctionType functionType) noexcept -> std::string {
        switch (functionType) {
            case FunctionType::SPHERE:   return "Sphere";
            case FunctionType::BOX:      return "Box";
            case FunctionType::CONE:     return "Cone";
            case FunctionType::CYLINDER: return "Cylinder";
            default: { qWarning("No matching implicit function type found"); return ""; }
        }
    }
    ENUM_GET_VALUES(FunctionType)

    struct TissueType {
        std::string Name;
        float Radiodensity = 0.0; // value on the Hounsfield scale

        auto
        operator==(const TissueType& other) const noexcept -> bool { return Name == other.Name && Radiodensity == other.Radiodensity; }

        friend auto operator<<(std::ostream& stream, const TissueType& type) -> std::ostream& {
            return stream << type.Name << ": ('" << type.Radiodensity << "')";
        }
    };

    static std::map<std::string, TissueType> TissueTypeMap {
            { "Air",             { "Air",            -1000.0F } },
            { "Lung",            { "Lung",            -600.0F } },
            { "Fat",             { "Fat",             -100.0F } },
            { "Water",           { "Water",              0.0F } },
            { "Muscle",          { "Muscle",            50.0F } },
            { "Soft Tissue",     { "Soft Tissue",      200.0F } },
            { "Cancellous Bone", { "Cancellous Bone",  350.0F } },
            { "Cortical Bone",   { "Cortical Bone",    800.0F } },
            { "Metal",           { "Metal",           3000.0F } }
    };

    [[nodiscard]] auto static
    GetTissueTypeByName(const std::string& tissueName) noexcept -> TissueType {
        if (auto search = TissueTypeMap.find(tissueName);
                search != TissueTypeMap.end())
            return search->second;

        qWarning("No tissue type with requested name present. Returning 'Air'");

        return TissueTypeMap.at("Air");
    }

    [[nodiscard]] auto static
    GetTissueTypeNames() noexcept -> QStringList {
        QStringList names;
        std::transform(TissueTypeMap.cbegin(), TissueTypeMap.cend(), std::back_inserter(names),
                       [](const auto& type) { return QString::fromStdString(type.first); });
        return names;
    }


    struct BasicStructureDataImpl {
        FunctionType FunctionType;
        QString TissueName;
        ShapeDataVariant Data;

        using Structure = BasicStructure;

        auto
        PopulateFromStructure(const Structure& structure) noexcept -> void;

        auto
        PopulateStructure(Structure& structure) const noexcept -> void;
    };

    class BasicStructureWidgetImpl : public QWidget {
    public:
        using Data = BasicStructureDataImpl;

        BasicStructureWidgetImpl();

        auto
        GetData() noexcept -> Data;

        auto
        Populate(const Data& data) noexcept -> void;

    private:
        auto
        UpdateFunctionParametersGroup() -> void;

        QFormLayout* Layout;
        QComboBox* TissueTypeComboBox;
        QComboBox* FunctionTypeComboBox;
        QGroupBox* SubTypeGroupBox;
        ShapeWidgetVariant SubTypeWidgetVariant;
    };
}

class BasicStructureData : public CtStructureBaseData<BasicStructureDetails::BasicStructureDataImpl> {};

class BasicStructureWidget : public CtStructureBaseWidget<BasicStructureDetails::BasicStructureWidgetImpl,
                                                          BasicStructureData> {
    Q_OBJECT
};



class BasicStructure : public CtStructureBase {
public:
    using FunctionType = BasicStructureDetails::FunctionType;
    using TissueType = BasicStructureDetails::TissueType;

    explicit BasicStructure(FunctionType functionType = FunctionType::SPHERE);
    explicit BasicStructure(auto&& shape) : Shape(std::move(shape)) {}
    explicit BasicStructure(const BasicStructureData& data);

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetFunctionType() const noexcept -> FunctionType;

    auto
    SetTissueType(TissueType tissueType) noexcept -> void;

    [[nodiscard]] auto
    GetTissueValue() const noexcept -> float;

    auto
    SetEvaluationBias(float evaluationBias) noexcept -> void;

    [[nodiscard]] inline auto
    FunctionValue(Point point) const -> float;

    [[nodiscard]] inline auto
    ClosestPointOnXYPlane(Point point) const -> std::optional<DoublePoint>;

    [[nodiscard]] auto
    GetId() const noexcept -> StructureId { return Id; }

    auto
    operator==(const BasicStructure& other) const -> bool { return Id == other.Id && Tissue == other.Tissue; }

private:
    friend struct EvaluateImplicitStructures;
    friend class BasicStructureDetails::BasicStructureDataImpl;

    StructureId Id = ++GlobalBasicStructureId;
    TissueType Tissue = BasicStructureDetails::GetTissueTypeByName("Air");
    ShapeVariant Shape;
    float EvaluationBias = 0.0F;

    static std::atomic<StructureId> GlobalBasicStructureId;
};

auto BasicStructure::FunctionValue(Point point) const -> float {
    return std::visit([this, &point](const auto& shape) {
        float const val = shape.EvaluateFunction(point);
        return std::min(val < 0 ? val + EvaluationBias
                                : val,
                        0.0F);
    }, Shape);
}

auto BasicStructure::ClosestPointOnXYPlane(Point point) const -> std::optional<DoublePoint> {
    return std::visit([&point](const auto& shape) { return shape.ClosestPointOnXYPlane(point); },
                      Shape);
}
