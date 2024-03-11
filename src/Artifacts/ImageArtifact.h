#pragma once

#include "Artifact.h"

class ImageArtifact : public Artifact {
public:
    vtkTypeMacro(ImageArtifact, Artifact);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type getArtifactType() const override;

    ImageArtifact(const ImageArtifact&) = delete;
    void operator=(const ImageArtifact&) = delete;

protected:
    ImageArtifact() = default;
    ~ImageArtifact() override = default;
};
