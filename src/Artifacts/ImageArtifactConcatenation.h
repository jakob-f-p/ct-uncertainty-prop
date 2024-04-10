#pragma once

#include <vtkNew.h>
#include <vtkObject.h>

class ImageArtifact;
class CompositeArtifact;

class ImageArtifactConcatenation : public vtkObject {
public:
    static ImageArtifactConcatenation* New();
    vtkTypeMacro(ImageArtifactConcatenation, vtkObject);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    bool ContainsImageArtifact(const ImageArtifact& imageArtifact);

    void AddImageArtifact(ImageArtifact& imageArtifact, CompositeArtifact* parent = nullptr);

    void RemoveImageArtifact(ImageArtifact& imageArtifact);

    CompositeArtifact& GetStart();

    ImageArtifactConcatenation(const ImageArtifactConcatenation&) = delete;
    void operator=(const ImageArtifactConcatenation&) = delete;

protected:
    ImageArtifactConcatenation();
    ~ImageArtifactConcatenation() override = default;

    vtkNew<CompositeArtifact> Start;
};
