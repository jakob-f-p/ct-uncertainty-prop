#pragma once

#include "Artifact.h"

#include <vtkWeakPointer.h>

class CtStructure;
class StructureArtifact;

class ArtifactStructureWrapper {
public:
    explicit ArtifactStructureWrapper(CtStructure& structure);
    ArtifactStructureWrapper(const ArtifactStructureWrapper&) = delete;
    ArtifactStructureWrapper(ArtifactStructureWrapper&&) = default;
    ArtifactStructureWrapper& operator=(const ArtifactStructureWrapper&) = delete;
    ArtifactStructureWrapper& operator=(ArtifactStructureWrapper&&) = default;

    [[nodiscard]] vtkMTimeType GetMTime() const;

    void AddStructureArtifact(StructureArtifact& structureArtifact);

    void RemoveStructureArtifact(StructureArtifact& structureArtifact);

    using TypeToStructureArtifactsMap = std::map<Artifact::SubType, std::vector<StructureArtifact*>>;
    TypeToStructureArtifactsMap GetStructureArtifactMap();

    void AddArtifactValuesAtPositionToMap(const double x[3], std::map<Artifact::SubType, float>& artifactValueMap);

private:
    friend class TreeStructureArtifactCollection;

    using StructureArtifactList = std::vector<vtkSmartPointer<StructureArtifact>>;

    vtkWeakPointer<CtStructure> Structure;
    StructureArtifactList StructureArtifacts;
};


class CtStructureTree;

class TreeStructureArtifactCollection : public vtkObject {
public:
    static TreeStructureArtifactCollection* New();
    vtkTypeMacro(TreeStructureArtifactCollection, vtkObject)

    void AddStructureArtifactList(CtStructure& structure);

    void RemoveStructureArtifactList(CtStructure& structure);

    vtkMTimeType GetMTime() override;

protected:
    TreeStructureArtifactCollection() = default;
    ~TreeStructureArtifactCollection() override = default;

private:
    using StructureArtifactLists = std::vector<ArtifactStructureWrapper>;
    StructureArtifactLists ArtifactLists;
};