#pragma once

#include "Artifact.h"

class QFormLayout;
class vtkImageAlgorithm;

class StructureArtifact : public Artifact {
public:
    vtkTypeMacro(StructureArtifact, Artifact)
    static StructureArtifact* NewStructureArtifact(SubType subType);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type GetArtifactType() const override;

    virtual float EvaluateAtPosition(const double x[3]) = 0;

    virtual bool IgnoreCompetingStructures() = 0;

//    virtual vtkSmartPointer<vtkImageAlgorithm> GetImageFilter() = 0;

    virtual void DeepCopy(StructureArtifact* source);

    StructureArtifact(const StructureArtifact&) = delete;
    void operator=(const StructureArtifact&) = delete;

protected:
    StructureArtifact() = default;
    ~StructureArtifact() override = default;
};



class StructureArtifactUi;

struct StructureArtifactData : ArtifactData<StructureArtifact, StructureArtifactData> {
    virtual ~StructureArtifactData() = default;

private:
    friend struct ArtifactData<StructureArtifact, StructureArtifactData>;
    friend struct ArtifactUi<StructureArtifactUi, StructureArtifactData>;

    static std::unique_ptr<StructureArtifactData> QVariantToData(const QVariant& variant);

    static QVariant DataToQVariant(const StructureArtifactData& data);

    static void AddDerivedData(const StructureArtifact& artifact, StructureArtifactData& data);

    static void SetDerivedData(StructureArtifact& artifact, const StructureArtifactData& data);

    static std::unique_ptr<StructureArtifactData> Create(Artifact::SubType subType);
};



class StructureArtifactUi : public ArtifactUi<StructureArtifactUi, StructureArtifactData> {
protected:
    friend struct ArtifactUi<StructureArtifactUi, StructureArtifactData>;

    static void AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType = Artifact::SubType::INVALID);

    static void AddDerivedWidgetsData(QWidget* widget, StructureArtifactData& data);

    static void SetDerivedWidgetsData(QWidget* widget, const StructureArtifactData& data);

    static std::vector<EnumString<Artifact::SubType>> GetSubTypeValues();
};
