#pragma once

#include "ImageArtifact.h"

#include <vtkSmartPointer.h>

class MergeParallelImageArtifactFilters;

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
    ENUM_GET_VALUES(CompositionType, true);

    vtkSetEnumMacro(CompType, CompositionType);
    vtkGetEnumMacro(CompType, CompositionType);

    bool ContainsImageArtifact(const ImageArtifact& artifact);

    void AddImageArtifact(ImageArtifact& artifact, int idx = -1);

    void RemoveImageArtifact(ImageArtifact& artifact);

    ImageArtifact* ChildArtifact(int idx);

    int GetChildIdx(ImageArtifact& artifact);

    int NumberOfChildren();

    void MoveChildImageArtifact(ImageArtifact* imageArtifact, int newIdx);

    auto AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& override;

    CompositeArtifact(const CompositeArtifact&) = delete;
    void operator=(const CompositeArtifact&) = delete;

protected:
    CompositeArtifact() = default;
    ~CompositeArtifact() override = default;

    friend struct CompositeArtifactData;

    std::vector<vtkSmartPointer<ImageArtifact>> ImageArtifacts;
    CompositionType CompType = INVALID;

    vtkSmartPointer<MergeParallelImageArtifactFilters> Filter;
};



struct CompositeArtifactData : ImageArtifactData {
    CompositeArtifact::CompositionType CompositionType = CompositeArtifact::INVALID;

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
