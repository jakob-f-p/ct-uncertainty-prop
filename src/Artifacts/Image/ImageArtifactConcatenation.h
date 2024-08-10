#pragma once

#include <functional>
#include <map>
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
//    ImageArtifactConcatenation(ImageArtifactConcatenation const& other);
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
    UpdateArtifactFilter() -> void;

    [[nodiscard]] auto
    GetFilterMTime() const noexcept -> vtkMTimeType;

    [[nodiscard]] auto
    GetStartFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetEndFilter() const -> vtkImageAlgorithm&;

private:
    friend class ImageArtifactsModel;
    friend class ImageArtifactsReadOnlyModel1;

    [[nodiscard]] auto
    GetStart() noexcept -> ImageArtifact&;

    [[nodiscard]] auto
    Get(uint16_t idx) -> ImageArtifact&;

    [[nodiscard]] auto
    IndexOf(ImageArtifact const& imageArtifact) const -> uint16_t;

    auto
    EmitEvent() -> void;

    auto
    EmitRemoveEvent(ImageArtifact* artifact) -> void;

    BeforeRemoveArtifactCallback BeforeRemoveCallback;
    std::unique_ptr<ImageArtifact> Start; // Composite
    vtkNew<PassThroughImageArtifactFilter> StartFilter;
    vtkNew<PassThroughImageArtifactFilter> EndFilter;
};
