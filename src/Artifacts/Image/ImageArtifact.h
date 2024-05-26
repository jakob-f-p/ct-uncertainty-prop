#pragma once

#include "CompositeImageArtifact.h"

#include <QWidget>

class ImageArtifactData;

class QFormLayout;

class ImageArtifact {
public:
    explicit ImageArtifact(ImageArtifactData const& data);
    ImageArtifact(BasicImageArtifact&& basicImageArtifact);
    ImageArtifact(CompositeImageArtifact&& compositeImageArtifact);
    ImageArtifact(ImageArtifact const& other) = default;

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

    [[nodiscard]] auto
    ContainsImageArtifact(ImageArtifact const& imageArtifact) -> bool;

    [[nodiscard]] auto
    GetParent() const -> ImageArtifact*;

    auto
    SetParent(ImageArtifact* parent) -> void;

    [[nodiscard]] auto
    ToComposite() -> CompositeImageArtifact&;

    [[nodiscard]] auto
    ToCompositeConst() const -> CompositeImageArtifact const&;

    [[nodiscard]] auto
    IsComposite() const noexcept -> bool;

    [[nodiscard]] auto
    NumberOfChildren() const noexcept -> uint16_t;

    [[nodiscard]] auto
    Get(uint16_t targetIdx, uint16_t& currentIdx) -> ImageArtifact*;

    [[nodiscard]] auto
    IndexOf(ImageArtifact const& imageArtifact, uint16_t& currentIdx) const -> int32_t;

    auto
    AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm&;

private:
    friend struct ImageArtifactData;

    using ImageArtifactVariant = std::variant<BasicImageArtifact, CompositeImageArtifact>;
    ImageArtifactVariant Artifact;
};


struct ImageArtifactData {
    using ImageArtifactDataVariant = std::variant<BasicImageArtifactData, CompositeImageArtifactData>;

    ImageArtifactData() = default;
    explicit ImageArtifactData(ImageArtifact const& artifact);
    explicit ImageArtifactData(BasicImageArtifactData&& data);
    explicit ImageArtifactData(CompositeImageArtifactData&& data);

    auto
    PopulateFromArtifact(ImageArtifact const& imageArtifact) noexcept -> void;

    auto
    PopulateArtifact(ImageArtifact& imageArtifact) const noexcept -> void;

    ImageArtifactDataVariant Data;
};

class ImageArtifactWidget : public QWidget {
    Q_OBJECT

public:
    enum struct Type : uint8_t {
        BASIC,
        COMPOSITE
    };
    Q_ENUM(Type);

    ImageArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> ImageArtifactData;

    auto
    Populate(ImageArtifactData const& data) noexcept -> void;

    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> ImageArtifactData { return FindWidget(widget).GetData(); }

    auto static
    SetWidgetData(QWidget* widget, ImageArtifactData const& data) -> void { FindWidget(widget).Populate(data); }

private:
    [[nodiscard]] auto static
    FindWidget(QWidget* widget) -> ImageArtifactWidget&;

    auto
    UpdateTypeWidget() noexcept -> void;

    [[nodiscard]] static auto
    TypeToString(Type type) noexcept -> std::string;

    using TypeWidgetVariant = std::variant<BasicImageArtifactWidget*, CompositeImageArtifactWidget*>;

    QFormLayout* Layout;
    QComboBox* TypeComboBox;
    TypeWidgetVariant TypeWidget;
};
