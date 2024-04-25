#pragma once

#include <map>
#include <memory>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

class CompositeImageArtifact;
class CtDataSource;
class ImageArtifact;
class ImageArtifactFilter;
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
    ~ImageArtifactConcatenation();

    [[nodiscard]] auto
    ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool;

    auto
    AddImageArtifact(ImageArtifact&& imageArtifact,
                     ImageArtifact* parent = nullptr,
                     int insertionIdx = -1) -> ImageArtifact&;

    auto
    RemoveImageArtifact(ImageArtifact& imageArtifact) -> void;

    void
    MoveChildImageArtifact(const ImageArtifact& imageArtifact, int newIdx);

    [[nodiscard]] auto
    GetStart() noexcept -> ImageArtifact&;

    [[nodiscard]] auto
    Get(uint16_t idx) -> ImageArtifact&;

    [[nodiscard]] auto
    IndexOf(const ImageArtifact& imageArtifact) const -> uint16_t;

    [[nodiscard]] auto
    GetArtifactFilter() const -> vtkImageAlgorithm&;

    auto
    UpdateArtifactFilter() -> void;

    using EventCallback = std::function<void()>;
    void AddEventCallback(void* receiver, EventCallback&& callback);

private:
    auto
    EmitEvent() -> void;

    std::unique_ptr<ImageArtifact> Start; // Composite
    vtkNew<CtDataSource> StartFilter;
    vtkNew<PassThroughImageArtifactFilter> EndFilter;

    std::map<void*, EventCallback> CallbackMap;
};
