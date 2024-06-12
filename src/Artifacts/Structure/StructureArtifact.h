#pragma once

#include "MotionArtifact.h"
#include "../../Utils/Enum.h"
#include "../../Utils/Types.h"

#include <QWidget>

#include <vtkTimeStamp.h>

#include <string>
#include <variant>

class NameLineEdit;

class QComboBox;
class QFormLayout;
class QGroupBox;

class vtkImageAlgorithm;


namespace StructureArtifactDetails {
    Q_NAMESPACE

    enum struct SubType : uint8_t {
        STREAKING = 0,
        METALLIC,
        MOTION
    };
    Q_ENUM_NS(SubType);

    [[nodiscard]] static auto
    SubTypeToString(SubType subType) noexcept -> std::string {
        switch (subType) {
            case SubType::STREAKING: return "Streaking";
            case SubType::METALLIC:  return "Metallic";
            case SubType::MOTION:    return "Motion";
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
    using StructureArtifactDataVariant = std::variant<MotionArtifactData>;

    QString Name;
    QString ViewName;
    StructureArtifactDataVariant Data;

    auto
    PopulateFromArtifact(const StructureArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(StructureArtifact& artifact) const noexcept -> void;
};


class StructureArtifact {
public:
    using StructureArtifactVariant = std::variant<MotionArtifact>;
    using SubType = StructureArtifactDetails::SubType;

    StructureArtifact() = default;
    StructureArtifact(StructureArtifact const&) = default;
    auto operator= (StructureArtifact const&) -> StructureArtifact& = delete;
    StructureArtifact(StructureArtifact&&) = default;
    auto operator= (StructureArtifact&&) -> StructureArtifact& = default;
    explicit StructureArtifact(StructureArtifactData const& data);
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
                       bool pointOccupiedByStructure,
                       float tissueValue,
                       StructureEvaluator const& structureEvaluator) const noexcept -> float {
        return std::visit([&](auto const& artifact)
                                  { return artifact.EvaluateAtPosition(point,
                                                                       pointOccupiedByStructure,
                                                                       tissueValue,
                                                                       structureEvaluator); },
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
    GetSubType(StructureArtifactVariant const& artifactVariant) noexcept -> SubType;

    std::string Name;
    StructureArtifactVariant Artifact;
};


class StructureArtifactWidget : public QWidget {
    Q_OBJECT

    using DataVariant = StructureArtifactData::StructureArtifactDataVariant;
    using StructureArtifactWidgetVariant = std::variant<MotionArtifactWidget*>;

public:
    StructureArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> StructureArtifactData;

    auto
    Populate(const StructureArtifactData& data) noexcept -> void;

private:
    auto
    UpdateSubTypeWidget() noexcept -> void;

    QFormLayout* Layout;
    NameLineEdit* NameEdit;
    QComboBox* SubTypeComboBox;
    QGroupBox* SubTypeGroupBox;
    StructureArtifactWidgetVariant WidgetVariant;
};
