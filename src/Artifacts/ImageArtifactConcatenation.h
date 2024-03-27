#pragma once

#include <vtkObject.h>

class ImageArtifact;
class ImageArtifactComposition;

class ImageArtifactConcatenation : public vtkObject {
public:
    static ImageArtifactConcatenation* New();
    vtkTypeMacro(ImageArtifactConcatenation, vtkObject);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    bool ContainsImageArtifact(const ImageArtifact& imageArtifact);

    void AddImageArtifact(ImageArtifact& imageArtifact, ImageArtifactComposition* parent = nullptr);

    void RemoveImageArtifact(ImageArtifact& imageArtifact);

    ImageArtifactComposition& GetStart();

    ImageArtifactConcatenation(const ImageArtifactConcatenation&) = delete;
    void operator=(const ImageArtifactConcatenation&) = delete;

protected:
    ImageArtifactConcatenation();
    ~ImageArtifactConcatenation() override;

    ImageArtifactComposition& Start;
};
