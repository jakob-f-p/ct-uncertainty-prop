#pragma once

#include "Artifact.h"

class ImageArtifactComposition;
struct ImageArtifactDetails;

class ImageArtifact : public Artifact {
public:
    vtkTypeMacro(ImageArtifact, Artifact)

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type GetArtifactType() const override;

    ImageArtifactComposition* GetParent();
    void SetParent(ImageArtifactComposition* parent);

    virtual QVariant Data() = 0;

    bool IsComposition() const;

    virtual ImageArtifactDetails GetImageArtifactEditWidgetData(QWidget* widget) const = 0;

    ImageArtifact(const ImageArtifact&) = delete;
    void operator=(const ImageArtifact&) = delete;

protected:
    ImageArtifact();
    ~ImageArtifact() override = default;

    ImageArtifactDetails GetImageArtifactDetails();
    std::string GetViewName();

    void SetChildEditWidgetData(QWidget* widget, const ArtifactDetails& artifactDetails) const override;
    void SetChildData(const ArtifactDetails& artifactDetails) override;

    virtual void SetImageArtifactChildEditWidgetData(QWidget* widget, const ImageArtifactDetails& details) const = 0;
    virtual void SetImageArtifactChildData(const ImageArtifactDetails& details) = 0;

    ImageArtifactComposition* Parent;
};
