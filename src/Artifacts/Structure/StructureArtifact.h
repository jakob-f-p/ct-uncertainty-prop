#pragma once

#include "MotionArtifact.h"
#include "MetalArtifact.h"
#include "WindmillArtifact.h"
#include "../../Utils/Enum.h"
#include "../../Utils/LinearAlgebraTypes.h"

#include <QWidget>

#include <vtkTimeStamp.h>

#include <string>
#include <variant>

class CtStructureTree;
class CtStructureVariant;

class NameLineEdit;

class QComboBox;
class QFormLayout;
class QGroupBox;

class vtkImageAlgorithm;


namespace StructureArtifactDetails {
    Q_NAMESPACE

    enum struct SubType : uint8_t {
        MOTION = 0,
        METAL,
        WINDMILL
    };
    Q_ENUM_NS(SubType);

    [[nodiscard]] static auto
    SubTypeToString(SubType subType) noexcept -> std::string {
        switch (subType) {
            case SubType::MOTION:   return "Motion";
            case SubType::METAL:    return "Metal";
            case SubType::WINDMILL: return "Windmill";
        }

        return "";
    }

    ENUM_GET_VALUES(SubType);

    [[nodiscard]] static consteval auto
    GetNumberOfSubTypeValues() -> int { return 3; };

    [[nodiscard]] static consteval auto
    GetSubTypeValuesConstEval() {
        std::array<SubType, GetNumberOfSubTypeValues()> values {};
        for (int i = 0; i < values.size(); i++)
            values[i] = static_cast<SubType>(i);

        return values;
    }
}


class StructureArtifact;

struct StructureArtifactData {
public:
    using StructureArtifactDataVariant = std::variant<MotionArtifactData, MetalArtifactData, WindmillArtifactData>;

    QString Name;
    QString ViewName;
    StructureArtifactDataVariant Data;

    auto
    PopulateFromArtifact(StructureArtifact const& artifact) noexcept -> void;

    auto
    PopulateArtifact(StructureArtifact& artifact) const noexcept -> void;
};


class StructureArtifact {
public:
    using StructureArtifactVariant = std::variant<MotionArtifact, MetalArtifact, WindmillArtifact>;
    using SubType = StructureArtifactDetails::SubType;

    StructureArtifact() = default;
    StructureArtifact(StructureArtifact const&) = default;
    auto operator= (StructureArtifact const&) -> StructureArtifact& = delete;
    StructureArtifact(StructureArtifact&&) = default;
    auto operator= (StructureArtifact&&) -> StructureArtifact& = default;
    explicit StructureArtifact(StructureArtifactData const& data);
    explicit StructureArtifact(MetalArtifact&& structureArtifactSubType)
            : Artifact(std::move(structureArtifactSubType)) {}
    explicit StructureArtifact(WindmillArtifact&& structureArtifactSubType)
            : Artifact(std::move(structureArtifactSubType)) {}
    explicit StructureArtifact(MotionArtifact&& structureArtifactSubType)
            : Artifact(std::move(structureArtifactSubType)) {}

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType {
        return std::visit([](auto const& artifact) { return artifact.GetMTime(); }, Artifact);
    }

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return Name; }

    auto
    SetName(const std::string& name) noexcept -> void {
        Name = name;
        std::visit([](auto& artifact) { artifact.Modified(); }, Artifact);
    }

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

    using StructureEvaluator = std::function<float(DoublePoint)>;

    [[nodiscard]] auto
    EvaluateAtPosition(DoublePoint const& point,
                       float maxRadiodensity,
                       bool pointOccupiedByStructure,
                       CtStructureTree const& structureTree,
                       CtStructureVariant const& structure,
                       std::array<double, 3> spacing) const noexcept -> float {
        return std::visit([&](auto const& artifact)
                                  { return artifact.EvaluateAtPosition(point,
                                                                       maxRadiodensity,
                                                                       pointOccupiedByStructure,
                                                                       structureTree,
                                                                       structure,
                                                                       spacing); },
                          Artifact);
    }

    [[nodiscard]] auto
    GetSubType() const noexcept -> SubType;

    [[nodiscard]] auto
    operator== (StructureArtifact const& other) const noexcept -> bool { return GetMTime() == other.GetMTime(); }

private:
    friend struct StructureArtifactData;
    friend class StructureArtifactWidget;

    [[nodiscard]] auto static
    GetSubType(StructureArtifactVariant const& artifactVariant) -> SubType;

    std::string Name;
    StructureArtifactVariant Artifact;
};


class StructureArtifactWidget : public QWidget {
    Q_OBJECT

    using DataVariant = StructureArtifactData::StructureArtifactDataVariant;
    using StructureArtifactWidgetVariant = std::variant<MotionArtifactWidget*,
                                                        MetalArtifactWidget*,
                                                        WindmillArtifactWidget*>;

public:
    StructureArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> StructureArtifactData;

    auto
    Populate(StructureArtifactData const& data) noexcept -> void;

private:
    auto
    UpdateSubTypeWidget() noexcept -> void;

    QFormLayout* Layout;
    NameLineEdit* NameEdit;
    QComboBox* SubTypeComboBox;
    QGroupBox* SubTypeGroupBox;
    StructureArtifactWidgetVariant WidgetVariant;
};
