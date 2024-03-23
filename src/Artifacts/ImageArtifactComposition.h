#pragma once

#include "ImageArtifact.h"

class ImageArtifactComposition : public ImageArtifact {
public:
    static ImageArtifactComposition* New();
    vtkTypeMacro(ImageArtifactComposition, ImageArtifact)

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    SubType GetArtifactSubType() const override;

    enum CompositionType {
        SEQUENTIAL,
        PARALLEL,
        INVALID
    };
    Q_ENUM(CompositionType);
    static std::string CompositionTypeToString(CompositionType compositionType);
    GET_ENUM_VALUES(CompositionType);
    vtkSetEnumMacro(CompType, CompositionType);

    bool ContainsImageArtifact(const ImageArtifact& artifact);

    void AddImageArtifact(ImageArtifact& artifact);

    void RemoveImageArtifact(ImageArtifact& artifact);

    ImageArtifact* ChildArtifact(int idx);

    int GetChildIdx(ImageArtifact& artifact);

    int NumberOfChildren();

    QVariant Data() override;

    ImageArtifactComposition(const ImageArtifactComposition&) = delete;
    void operator=(const ImageArtifactComposition&) = delete;

protected:
    ImageArtifactComposition();
    ~ImageArtifactComposition() override;

    std::vector<ImageArtifact*> ImageArtifacts;
    CompositionType CompType;
};

struct ImageArtifactCompositionDetails {
    ImageArtifactComposition::CompositionType CompositionType = ImageArtifactComposition::INVALID;
};
