#pragma once

#include "BasicStructures.h"
#include "CtStructure.h"

class BasicStructure;

class QComboBox;
class QFormLayout;
class QGroupBox;

template<class... Ts>
struct Overload : Ts... { using Ts::operator()...; };

namespace BasicStructureDetails {
    Q_NAMESPACE

    enum struct FunctionType : uint8_t {
        SPHERE,
        BOX,
        CONE
    };
    Q_ENUM_NS(FunctionType);

    [[nodiscard]] auto static
    FunctionTypeToString(FunctionType functionType) noexcept -> std::string {
        switch (functionType) {
            case FunctionType::SPHERE: return "Sphere";
            case FunctionType::BOX:    return "Box";
            case FunctionType::CONE:   return "Cone";
            default: { qWarning("No matching implicit function type found"); return ""; }
        }
    }
    ENUM_GET_VALUES(FunctionType, false)

    struct TissueType {
        std::string Name;
        float CtNumber = 0.0; // value on the Hounsfield scale

        auto
        operator==(const TissueType& other) const noexcept -> bool { return Name == other.Name && CtNumber == other.CtNumber; }

        friend auto operator<<(std::ostream& stream, const TissueType& type) -> std::ostream& {
            return stream << type.Name << ": ('" << type.CtNumber << "')";
        }
    };

    static std::map<std::string, TissueType> TissueTypeMap {
            { "Air",             { "Air",            -1000.0F } },
            { "Fat",             { "Fat",             -100.0F } },
            { "Water",           { "Water",              0.0F } },
            { "Soft Tissue",     { "Soft Tissue",      200.0F } },
            { "Cancellous Bone", { "Cancellous Bone",  350.0F } },
            { "Cortical Bone",   { "Cortical Bone",    800.0F } },
            { "Metal",           { "Metal",          15000.0F } }
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
        BasicStructureSubTypeDataVariant Data;

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
        AddData(Data& data) noexcept -> void;

        auto
        Populate(const Data& data) noexcept -> void;

    private:
        auto
        UpdateFunctionParametersGroup() -> void;

        QFormLayout* Layout;
        QComboBox* TissueTypeComboBox;
        QComboBox* FunctionTypeComboBox;
        QGroupBox* SubTypeGroupBox;
        BasicStructureSubTypeWidgetVariant SubTypeWidgetVariant;
    };
}

class BasicStructureData : public CtStructureBaseData<BasicStructureDetails::BasicStructureDataImpl> {};

class BasicStructureWidget : public CtStructureBaseWidget<BasicStructureDetails::BasicStructureWidgetImpl,
                                                          BasicStructureData> {
    Q_OBJECT

public:
    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> BasicStructureData { return FindWidget(widget).GetData(); }

    auto static
    SetWidgetData(QWidget* widget, const BasicStructureData& data) -> void { FindWidget(widget).Populate(data); }

private:
    [[nodiscard]] auto static
    FindWidget(QWidget* widget) -> BasicStructureWidget& {
        if (!widget)
            throw std::runtime_error("Given widget must not be nullptr");

        auto* basicStructureWidget = widget->findChild<BasicStructureWidget*>();

        if (!basicStructureWidget)
            throw std::runtime_error("No basic structure widget contained in given widget");

        return *basicStructureWidget;
    }
};

using BasicStructureDetails::FunctionType;
using BasicStructureDetails::TissueType;



class BasicStructure : public CtStructureBase {
public:
    using Data = BasicStructureData;

    explicit BasicStructure(FunctionType functionType = FunctionType::SPHERE);
    explicit BasicStructure(const BasicStructureData& data);

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetFunctionType() const noexcept -> FunctionType;

    auto
    SetTissueType(TissueType tissueType) noexcept -> void;

    [[nodiscard]] inline auto
    FunctionValue(Point point) const -> float;

    [[nodiscard]] auto
    GetData() const noexcept -> Data;

    auto
    SetData(const Data& data) noexcept -> void;

    auto
    operator==(const BasicStructure& other) const -> bool { return Id == other.Id && Tissue == other.Tissue; }

    using ShapeVariant = std::variant<Sphere, Box>;

private:
    friend struct EvaluateImplicitStructures;
    friend class BasicStructureDetails::BasicStructureDataImpl;

    StructureId Id = ++GlobalBasicStructureId;
    TissueType Tissue = BasicStructureDetails::GetTissueTypeByName("Air");
    ShapeVariant Shape;

    static std::atomic<StructureId> GlobalBasicStructureId;
};

auto BasicStructure::FunctionValue(Point point) const -> float {
    const Point transformedPoint = Transform.TransformPoint(point);

    return std::visit([&](const auto& shape) { return shape.EvaluateFunction(transformedPoint); }, Shape);
}
