#pragma once

#include "ImageArtifactBase.h"

#include "GaussianArtifact.h"
#include "RingArtifact.h"
#include "SaltPepperArtifact.h"

#include <QMetaObject>

#include <string>

class BasicImageArtifact;

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

    using BasicImageArtifactWidgetVariant = std::variant<GaussianArtifactWidget*, SaltPepperArtifactWidget*, RingArtifactWidget*>;

    struct BasicImageArtifactDataVariant : std::variant<GaussianArtifactData, SaltPepperArtifactData, RingArtifactData> {
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
        UpdateSubTypeWidget() noexcept -> void;

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
    using BasicImageArtifactVariant = std::variant<GaussianArtifact, SaltPepperArtifact, RingArtifact>;

    explicit BasicImageArtifact(const BasicImageArtifactData& data);
    explicit BasicImageArtifact(SubType subType = SubType::GAUSSIAN);
    explicit BasicImageArtifact(auto&& basicArtifact) : Artifact(std::move(basicArtifact)) {}

    [[nodiscard]] auto
    GetSubType() const noexcept -> SubType;

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

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
        public ImageArtifactBaseDetails::ImageArtifactBaseData<BasicImageArtifactDetails::BasicImageArtifactDataVariant> {};

struct BasicImageArtifactWidget
        : public ImageArtifactBaseDetails::ImageArtifactBaseWidget<BasicImageArtifactDetails::BasicImageArtifactWidgetImpl,
                                                                   BasicImageArtifactData> {
    Q_OBJECT
};
