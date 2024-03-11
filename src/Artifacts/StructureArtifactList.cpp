#include <vtkObjectFactory.h>
#include <vtkSetGet.h>

#include "StructureArtifactList.h"

vtkStandardNewMacro(StructureArtifactList)

void StructureArtifactList::AddStructureArtifact(StructureArtifact* structureArtifact) {
    if (!structureArtifact) {
        vtkErrorMacro("Structure artifact to add was nullptr");
        return;
    }

    Artifacts.push_back(structureArtifact);

    std::sort(Artifacts.begin(), Artifacts.end(), [](StructureArtifact* a, StructureArtifact* b) {
        return a && b && a->GetArtifactSubType() < b->GetArtifactSubType();
    });

    structureArtifact->Register(this);
}

StructureArtifactList::TypeToStructureArtifactsMap StructureArtifactList::GetStructureArtifactMap() {
    std::vector<Artifact::SubType> structureArtifactTypes = Artifact::GetStructureArtifactTypes();

    TypeToStructureArtifactsMap map;
    for (const auto &subType: structureArtifactTypes) {
        map[subType] = std::vector<StructureArtifact*>();
    }

    for (const auto &artifact: Artifacts) {
        map[artifact->GetArtifactSubType()].push_back(artifact);
    }

    return map;
}

void StructureArtifactList::AddArtifactValuesAtPositionToMap(const double x[3],
                                                             std::map<Artifact::SubType, float>& artifactValueMap) {
    for (const auto &artifact: Artifacts) {
        artifactValueMap[artifact->GetArtifactSubType()] += artifact->EvaluateAtPosition(x);
    }
}
