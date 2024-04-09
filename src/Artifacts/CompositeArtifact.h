#pragma once

#include "ImageArtifact.h"

class CompositeArtifact : public ImageArtifact {
    Q_GADGET

public:
    static CompositeArtifact* New();
    vtkTypeMacro(CompositeArtifact, ImageArtifact)

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

    void MoveChildImageArtifact(ImageArtifact* imageArtifact, int newIdx);

    CompositeArtifact(const CompositeArtifact&) = delete;
    void operator=(const CompositeArtifact&) = delete;

protected:
    CompositeArtifact();
    ~CompositeArtifact() override;

    friend struct CompositeArtifactData;

    std::vector<ImageArtifact*> ImageArtifacts;
    CompositionType CompType;
};



struct CompositeArtifactData : ImageArtifactData {
    struct CompositeData {
        CompositeArtifact::CompositionType CompositionType = CompositeArtifact::INVALID;
    };
    CompositeData Composite;

    ~CompositeArtifactData() override = default;

protected:
    friend struct ImageArtifactData;

    void AddSubTypeData(const ImageArtifact& imageArtifact) override;

    void SetSubTypeData(ImageArtifact& imageArtifact) const override;
};



class CompositeArtifactUi : public ImageArtifactUi {
protected:
    friend struct ImageArtifactUi;

    static void AddSubTypeWidgets(QFormLayout* fLayout);

    static void AddSubTypeWidgetsData(QWidget* widget, CompositeArtifactData& data);

    static void SetSubTypeWidgetsData(QWidget* widget, const CompositeArtifactData& data);

private:
    static const QString CompTypeComboBoxObjectName;
};
