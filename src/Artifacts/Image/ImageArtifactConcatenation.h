#pragma once

#include <functional>
#include <memory>

#include <vtkNew.h>
#include <vtkType.h>

class ImageArtifact;
class PassThroughImageArtifactFilter;
class vtkImageAlgorithm;


class ImageArtifactConcatenation {
public:
    using BeforeRemoveArtifactCallback = std::function<void(ImageArtifact&)>;

    explicit ImageArtifactConcatenation(BeforeRemoveArtifactCallback callback = [](ImageArtifact&){}) noexcept;
    ~ImageArtifactConcatenation();

    [[nodiscard]] auto
    ContainsImageArtifact(ImageArtifact const& imageArtifact) const noexcept -> bool;

    auto
    AddImageArtifact(ImageArtifact&& imageArtifact,
                     ImageArtifact* parent = nullptr,
                     int insertionIdx = -1) -> ImageArtifact&;

    auto
    RemoveImageArtifact(ImageArtifact& imageArtifact) -> void;

    void
    MoveChildImageArtifact(ImageArtifact const& imageArtifact, int newIdx);

    auto
    UpdateArtifactFilter() const -> void;

    [[nodiscard]] auto
    GetFilterMTime() const noexcept -> vtkMTimeType;

    [[nodiscard]] auto
    GetStartFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetEndFilter() const -> vtkImageAlgorithm&;

    using ConcatenationEventCallback = std::function<void()>;
    auto
    AddConcatenationEventCallback(ConcatenationEventCallback&& callback) -> void;

private:
    friend class ImageArtifactsModel;
    friend class ImageArtifactsReadOnlyModel1;

    [[nodiscard]] auto
    GetStart() const noexcept -> ImageArtifact&;

    [[nodiscard]] auto
    Get(uint16_t idx) const -> ImageArtifact&;

    [[nodiscard]] auto
    IndexOf(ImageArtifact const& imageArtifact) const -> uint16_t;

    auto
    EmitEvent() const -> void;

    BeforeRemoveArtifactCallback BeforeRemoveCallback;
    std::vector<ConcatenationEventCallback> Callbacks;

    std::unique_ptr<ImageArtifact> Start; // Composite
    vtkNew<PassThroughImageArtifactFilter> StartFilter;
    vtkNew<PassThroughImageArtifactFilter> EndFilter;
};
