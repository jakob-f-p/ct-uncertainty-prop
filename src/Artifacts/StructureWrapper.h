#pragma once

#include "Artifact.h"

#include <vtkWeakPointer.h>

class CtStructure;
class StructureArtifact;

class ArtifactStructureWrapper : public vtkObject {
public:
    static ArtifactStructureWrapper* New();
    vtkTypeMacro(ArtifactStructureWrapper, vtkObject)

    void SetCtStructure(CtStructure& structure);

    vtkMTimeType GetMTime() override;

    [[nodiscard]] StructureArtifact& Get(int idx);

    [[nodiscard]] int GetNumberOfArtifacts() const;

    void AddStructureArtifact(StructureArtifact& structureArtifact, int insertionIdx = -1);

    void RemoveStructureArtifact(StructureArtifact& structureArtifact);

    void MoveStructureArtifact(StructureArtifact* artifact, int newIdx);

    using TypeToStructureArtifactsMap = std::map<Artifact::SubType, std::vector<StructureArtifact*>>;
    TypeToStructureArtifactsMap GetStructureArtifactMap();

    void AddArtifactValuesAtPositionToMap(const double x[3], std::map<Artifact::SubType, float>& artifactValueMap);

    [[nodiscard]] std::string GetViewName() const;

private:
    ArtifactStructureWrapper() = default;
    ~ArtifactStructureWrapper() override = default;

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

    vtkMTimeType GetMTime() override;

    ArtifactStructureWrapper* GetForCtStructure(const CtStructure& structure);

    void AddStructureArtifactList(CtStructure& structure);

    void RemoveStructureArtifactList(CtStructure& structure);

protected:
    TreeStructureArtifactCollection() = default;
    ~TreeStructureArtifactCollection() override = default;

private:
    using StructureArtifactLists = std::vector<vtkSmartPointer<ArtifactStructureWrapper>>;
    StructureArtifactLists ArtifactLists;
};