#pragma once

#include <functional>
#include <map>
#include <memory>

#include <vtkNew.h>

class ImageArtifact;
class PassThroughImageArtifactFilter;
class vtkImageAlgorithm;

enum struct ImageArtifactConcatenationEventType : uint8_t {
    Add,
    Remove,
    Move,
    Edit
};

class ImageArtifactConcatenation;

struct ImageArtifactConcatenationEvent {
    ImageArtifactConcatenationEventType Type;
    ImageArtifactConcatenation* Emitter;
};

class ImageArtifactConcatenation {
public:
    ImageArtifactConcatenation() noexcept;
    ImageArtifactConcatenation(ImageArtifactConcatenation const& other);
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
    GetStartFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetEndFilter() const -> vtkImageAlgorithm&;

    using EventCallback = std::function<void()>;
    void AddEventCallback(void* receiver, EventCallback&& callback);

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

    std::unique_ptr<ImageArtifact> Start; // Composite
    vtkNew<PassThroughImageArtifactFilter> StartFilter;
    vtkNew<PassThroughImageArtifactFilter> EndFilter;

    std::map<void*, EventCallback> CallbackMap;
};
