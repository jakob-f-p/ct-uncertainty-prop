#pragma once

#include "MotionArtifact.h"
#include "../Enum.h"

#include <QWidget>

#include <vtkTimeStamp.h>

#include <string>
#include <variant>

class NameLineEdit;

class QComboBox;
class QFormLayout;
class QGroupBox;

class vtkImageAlgorithm;


using FloatPoint = std::array<float, 3>;

using StructureArtifactVariant = std::variant<MotionArtifact>;
using StructureArtifactDataVariant = std::variant<MotionArtifactData>;
using StructureArtifactWidgetVariant = std::variant<MotionArtifactWidget*>;

namespace StructureArtifactDetails {
    Q_NAMESPACE

    enum struct SubType : uint8_t {
        STREAKING,
        METALLIC,
        MOTION,
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
}


class StructureArtifact;

struct StructureArtifactData {
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
    using SubType = StructureArtifactDetails::SubType;

    explicit StructureArtifact(SubType subType = SubType::MOTION);
    explicit StructureArtifact(StructureArtifactData const& data);

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return MTime.GetMTime(); }

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return Name; }

    auto
    SetName(const std::string& name) noexcept -> void { Name = name; MTime.Modified(); }

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    EvaluateAtPosition(const FloatPoint& point) -> float;

    [[nodiscard]] auto
    GetSubType() const noexcept -> SubType;

private:
    friend struct StructureArtifactData;
    friend class StructureArtifactWidget;

    [[nodiscard]] auto static
    GetSubType(const StructureArtifactVariant& artifactVariant) noexcept -> SubType;

    std::string Name;
    StructureArtifactVariant Artifact;
    vtkTimeStamp MTime;
};


class StructureArtifactWidget : public QWidget {
    Q_OBJECT

public:
    StructureArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> StructureArtifactData;

    auto
    Populate(const StructureArtifactData& data) noexcept -> void;

    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> StructureArtifactData { return FindWidget(widget).GetData(); }

    auto static
    SetWidgetData(QWidget* widget, const StructureArtifactData& data) -> void { FindWidget(widget).Populate(data); }

private:
    [[nodiscard]] auto static
    FindWidget(QWidget* widget) -> StructureArtifactWidget&;

    auto
    UpdateSubTypeWidget() noexcept -> void;

    QFormLayout* Layout;
    NameLineEdit* NameEdit;
    QComboBox* SubTypeComboBox;
    QGroupBox* SubTypeGroupBox;
    StructureArtifactWidgetVariant WidgetVariant;
};
