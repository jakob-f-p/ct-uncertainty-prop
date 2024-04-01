#pragma once

#include "Artifact.h"

#include <QFormLayout>

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

    ImageArtifact(const ImageArtifact&) = delete;
    void operator=(const ImageArtifact&) = delete;

protected:
    ImageArtifact();

    friend struct ImageArtifactData;

    std::string GetViewName() const;

    CompositeArtifact* Parent;
};



class ImageArtifactUi;

struct ImageArtifactData : ArtifactData<ImageArtifact, ImageArtifactData> {
    QString ViewName;

    static std::unique_ptr<ImageArtifactData> FromQVariant(const QVariant& variant);
    static QVariant ToQVariant(const ImageArtifactData& data);

    ~ImageArtifactData() override = default;

protected:
    friend struct ArtifactData<ImageArtifact, ImageArtifactData>;
    friend struct ArtifactUi<ImageArtifactUi, ImageArtifactData>;

    static void AddDerivedData(const ImageArtifact& artifact, ImageArtifactData& data);

    static void SetDerivedData(ImageArtifact& artifact, const ImageArtifactData& data);

    static std::unique_ptr<ImageArtifactData> GetEmpty(const ImageArtifact& artifact);

    static std::unique_ptr<ImageArtifactData> GetEmpty(Artifact::SubType subType);
};



class ImageArtifactUi : public ArtifactUi<ImageArtifactUi, ImageArtifactData> {
protected:
    friend struct ArtifactUi<ImageArtifactUi, ImageArtifactData>;

    static void AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType = Artifact::SubType::INVALID);

    static void AddDerivedWidgetsData(QWidget* widget, ImageArtifactData& data);

    static void SetDerivedWidgetsData(QWidget* widget, const ImageArtifactData& data);

    static std::vector<EnumString<Artifact::SubType>> GetSubTypeValues();
};
