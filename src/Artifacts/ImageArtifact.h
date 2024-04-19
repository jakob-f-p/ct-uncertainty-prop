#pragma once

#include "Artifact.h"

class QFormLayout;
class vtkImageAlgorithm;

class CompositeArtifact;

class ImageArtifact : public Artifact {
public:
    vtkTypeMacro(ImageArtifact, Artifact)

    ~ImageArtifact() override = default;

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type GetArtifactType() const override;

    CompositeArtifact* GetParent();
    void SetParent(CompositeArtifact* parent);

    bool IsComposition() const;

    virtual auto
    AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& = 0;

    ImageArtifact(const ImageArtifact&) = delete;
    void operator=(const ImageArtifact&) = delete;

protected:
    ImageArtifact();

    friend struct ImageArtifactData;

    CompositeArtifact* Parent;
};



class ImageArtifactUi;

struct ImageArtifactData : ArtifactData<ImageArtifact, ImageArtifactData> {
    virtual ~ImageArtifactData() = default;

private:
    friend struct ArtifactData<ImageArtifact, ImageArtifactData>;
    friend struct ArtifactUi<ImageArtifactUi, ImageArtifactData>;

    static auto QVariantToData(const QVariant& variant) -> std::unique_ptr<ImageArtifactData>;

    static auto DataToQVariant(const ImageArtifactData& data) -> QVariant;

    static void AddDerivedData(const ImageArtifact& artifact, ImageArtifactData& data);

    static void SetDerivedData(ImageArtifact& artifact, const ImageArtifactData& data);

    static auto Create(Artifact::SubType subType) -> std::unique_ptr<ImageArtifactData>;
};



class ImageArtifactUi : public ArtifactUi<ImageArtifactUi, ImageArtifactData> {
protected:
    friend struct ArtifactUi<ImageArtifactUi, ImageArtifactData>;

    static void AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType = Artifact::SubType::INVALID);

    static void AddDerivedWidgetsData(QWidget* widget, ImageArtifactData& data);

    static void SetDerivedWidgetsData(QWidget* widget, const ImageArtifactData& data);

    static auto GetSubTypeValues() -> std::vector<EnumString<Artifact::SubType>>;
};
