#pragma once

#include "ImageArtifact.h"

class ImageArtifactComposition : public ImageArtifact {
    Q_GADGET

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
    GET_ENUM_VALUES(CompositionType, true);
    vtkSetEnumMacro(CompType, CompositionType);
    vtkGetEnumMacro(CompType, CompositionType);

    bool ContainsImageArtifact(const ImageArtifact& artifact);

    void AddImageArtifact(ImageArtifact& artifact, int idx = -1);

    void RemoveImageArtifact(ImageArtifact& artifact);

    ImageArtifact* ChildArtifact(int idx);

    int GetChildIdx(ImageArtifact& artifact);

    int NumberOfChildren();

    QVariant Data() override;

    ImageArtifactDetails GetImageArtifactEditWidgetData(QWidget* widget) const override;

    void MoveChildImageArtifact(ImageArtifact* imageArtifact, int newIdx);

    ImageArtifactComposition(const ImageArtifactComposition&) = delete;
    void operator=(const ImageArtifactComposition&) = delete;

protected:
    ImageArtifactComposition();
    ~ImageArtifactComposition() override;

    QWidget* GetChildEditWidget() const override;
    void SetImageArtifactChildEditWidgetData(QWidget* widget, const ImageArtifactDetails& details) const override;
    void SetImageArtifactChildData(const ImageArtifactDetails& details) override;

    std::vector<ImageArtifact*> ImageArtifacts;
    CompositionType CompType;

    QString CompTypeComboBoxObjectName;
};

struct ImageArtifactCompositionDetails {
    ImageArtifactComposition::CompositionType CompositionType = ImageArtifactComposition::INVALID;
};
