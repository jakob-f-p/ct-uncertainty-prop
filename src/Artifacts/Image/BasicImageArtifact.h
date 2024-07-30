#pragma once

#include "ImageArtifactBase.h"

#include "Artifacts/CuppingArtifact.h"
#include "Artifacts/GaussianArtifact.h"
#include "Artifacts/RingArtifact.h"
#include "Artifacts/SaltPepperArtifact.h"
#include "Artifacts/StairStepArtifact.h"
#include "Artifacts/WindMillArtifact.h"
#include "../Types.h"

#include <QMetaObject>

#include <string>

class BasicImageArtifact;

#define BASIC_IMAGE_ARTIFACT_TYPES \
GaussianArtifact, SaltPepperArtifact, RingArtifact, CuppingArtifact, WindMillArtifact, StairStepArtifact

namespace BasicImageArtifactDetails {
    Q_NAMESPACE

    enum struct SubType : uint8_t {
        GAUSSIAN,
        SALT_PEPPER,
        RING,
        CUPPING,
        WIND_MILL,
        STAIR_STEP,
        STREAKING
    };
    Q_ENUM_NS(SubType);

    [[nodiscard]] static auto
    SubTypeToString(SubType subType) noexcept -> std::string {
        switch (subType) {
            case SubType::GAUSSIAN:    return "Gaussian";
            case SubType::SALT_PEPPER: return "Salt and Pepper";
            case SubType::RING:        return "Ring";
            case SubType::CUPPING:     return "Cupping";
            case SubType::WIND_MILL:   return "Wind Mill";
            case SubType::STAIR_STEP:  return "Stair Step";
            case SubType::STREAKING:   return "Streaking";
        }
        return "";
    }

    ENUM_GET_VALUES(SubType);



    using BasicImageArtifactWidgetVariant = WidgetPointerVariant<BASIC_IMAGE_ARTIFACT_TYPES>;

    struct BasicImageArtifactDataVariant : DataVariant<BASIC_IMAGE_ARTIFACT_TYPES> {
        using Artifact = BasicImageArtifact;

        auto
        PopulateFromArtifact(const BasicImageArtifact& artifact) noexcept -> void;

        auto
        PopulateArtifact(BasicImageArtifact& artifact) const noexcept -> void;
    };

    class BasicImageArtifactWidgetImpl : public QWidget {
    public:
        using Data = BasicImageArtifactDataVariant;

        BasicImageArtifactWidgetImpl();

        [[nodiscard]] auto
        GetData() const noexcept -> Data;

        auto
        Populate(const Data& data) noexcept -> void;

    private:
        auto
        UpdateSubTypeWidget() -> void;

        QFormLayout* Layout;
        QComboBox* SubTypeComboBox;
        QGroupBox* SubTypeGroupBox;
        BasicImageArtifactWidgetVariant SubTypeWidgetVariant;
    };
}

class BasicImageArtifactData;

class BasicImageArtifact : public ImageArtifactBaseDetails::ImageArtifactBase {
public:
    using SubType = BasicImageArtifactDetails::SubType;
    using BasicImageArtifactVariant = std::variant<BASIC_IMAGE_ARTIFACT_TYPES>;

    BasicImageArtifact() = default;
    BasicImageArtifact(BasicImageArtifact const& other) = default;
    BasicImageArtifact(BasicImageArtifact&& other) = default;
    explicit BasicImageArtifact(BasicImageArtifactData const& data);
    explicit BasicImageArtifact(auto&& basicArtifact) :
            Artifact(std::forward<decltype(basicArtifact)>(basicArtifact)) {}

    [[nodiscard]] auto
    GetSubType() const noexcept -> SubType;

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

    auto
    AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm&;

private:
    [[nodiscard]] auto static
    GetSubType(const BasicImageArtifactVariant& artifact) noexcept -> SubType;

    friend struct BasicImageArtifactDetails::BasicImageArtifactDataVariant;
    friend struct BasicImageArtifactDetails::BasicImageArtifactWidgetImpl;

    BasicImageArtifactVariant Artifact;
};

struct BasicImageArtifactData :
        public ImageArtifactBaseDetails::ImageArtifactBaseData<
                BasicImageArtifactDetails::BasicImageArtifactDataVariant> {};

struct BasicImageArtifactWidget
        : public ImageArtifactBaseDetails::ImageArtifactBaseWidget<
                BasicImageArtifactDetails::BasicImageArtifactWidgetImpl,
                BasicImageArtifactData> {
    Q_OBJECT
};

#undef BASIC_IMAGE_ARTIFACT_TYPES
