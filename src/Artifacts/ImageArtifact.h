#pragma once

#include "Artifact.h"

class ImageArtifactComposition;
class ImageArtifactDetails;

class ImageArtifact : public Artifact {
public:
    vtkTypeMacro(ImageArtifact, Artifact)

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type GetArtifactType() const override;

    ImageArtifactComposition* GetParent();

    virtual QVariant Data() = 0;

    ImageArtifact(const ImageArtifact&) = delete;
    void operator=(const ImageArtifact&) = delete;

protected:
    ImageArtifact();
    ~ImageArtifact() override = default;

    ImageArtifactDetails GetImageArtifactDetails();

    ImageArtifactComposition* Parent;
};
