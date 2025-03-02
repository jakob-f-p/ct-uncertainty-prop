#pragma once

#include "BasicImageArtifact.h"
#include "../../Utils/Enum.h"

#include <QMetaEnum>
#include <QMetaObject>
#include <QWidget>

#include <vtkNew.h>

class MergeParallelImageArtifactFilters;

class QComboBox;
class QFormLayout;

class vtkImageAlgorithm;

namespace CompositeImageArtifactDetails {
    Q_NAMESPACE

    enum struct CompositionType : uint8_t {
        SEQUENTIAL,
        PARALLEL
    };
    Q_ENUM_NS(CompositionType);

    [[nodiscard]] static auto
    CompositionTypeToString(CompositionType compositionType) noexcept -> std::string {
        switch (compositionType) {
            case CompositionType::SEQUENTIAL: return "Sequential";
            case CompositionType::PARALLEL:   return "Parallel";
        }

        return "";
    }

    ENUM_GET_VALUES(CompositionType);

    struct CompositeImageArtifactData {
        CompositionType CompositionType = CompositionType::SEQUENTIAL;

        using Artifact = CompositeImageArtifact;

        auto
        PopulateFromArtifact(const CompositeImageArtifact& artifact) noexcept -> void;

        auto
        PopulateArtifact(CompositeImageArtifact& artifact) const noexcept -> void;
    };



    class CompositeImageArtifactWidgetImpl : public QWidget {
    public:
        using Data = CompositeImageArtifactData;

        CompositeImageArtifactWidgetImpl();

        [[nodiscard]] auto
        GetData() const noexcept -> CompositeImageArtifactData;

        auto
        Populate(const CompositeImageArtifactData& data) const noexcept -> void;

    private:
        QComboBox* CompositionTypeComboBox;
    };
}

struct CompositeImageArtifactData :
        ImageArtifactBaseDetails::ImageArtifactBaseData<CompositeImageArtifactDetails::CompositeImageArtifactData> {};

struct CompositeImageArtifactWidget :
        ImageArtifactBaseDetails::ImageArtifactBaseWidget<CompositeImageArtifactDetails::CompositeImageArtifactWidgetImpl,
                                                                 CompositeImageArtifactData> {
    Q_OBJECT
};


class ImageArtifact;

class CompositeImageArtifact : public ImageArtifactBaseDetails::ImageArtifactBase {
public:
    using CompositionType = CompositeImageArtifactDetails::CompositionType;

    explicit CompositeImageArtifact(CompositeImageArtifactData const& data);
    explicit CompositeImageArtifact(CompositionType compositionType = CompositionType::SEQUENTIAL);
    CompositeImageArtifact(CompositeImageArtifact const& other);
    CompositeImageArtifact(CompositeImageArtifact&&) noexcept;
    auto operator= (CompositeImageArtifact const&) -> CompositeImageArtifact& = delete;
    auto operator= (CompositeImageArtifact&&) noexcept -> CompositeImageArtifact&;
    ~CompositeImageArtifact();

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    [[nodiscard]] static auto
    GetProperties() noexcept -> PipelineParameterProperties;

    auto
    SetCompositionType(CompositionType compositionType) noexcept -> void { CompType = compositionType; }

    [[nodiscard]] auto
    GetCompositionType() const noexcept -> CompositionType { return CompType; }

    [[nodiscard]] auto
    ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool;

    auto
    AddImageArtifact(ImageArtifact&& artifact, int idx = -1) -> ImageArtifact&;

    void RemoveImageArtifact(const ImageArtifact& imageArtifact);

    void MoveChildImageArtifact(const ImageArtifact& imageArtifact, int newIdx);

    [[nodiscard]] auto
    NumberOfChildren() const noexcept -> uint8_t;

    [[nodiscard]] auto
    ChildArtifact(uint8_t idx) const -> ImageArtifact&;

    [[nodiscard]] auto
    GetChildIdx(const ImageArtifact& imageArtifact) const -> uint8_t;

    [[nodiscard]] auto
    Get(uint16_t targetIdx, uint16_t& currentIdx) const -> ImageArtifact*;

    [[nodiscard]] auto
    IndexOf(const ImageArtifact& imageArtifact, uint16_t& currentIdx) const -> int32_t;

    auto AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) const -> vtkImageAlgorithm&;

private:
    friend struct CompositeImageArtifactDetails::CompositeImageArtifactData;
    friend class ImageArtifactConcatenation;

    std::vector<std::unique_ptr<ImageArtifact>> ImageArtifacts;
    CompositionType CompType = CompositionType::SEQUENTIAL;

    vtkNew<MergeParallelImageArtifactFilters> Filter;
};


