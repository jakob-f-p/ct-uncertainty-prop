#include "StructureWrapper.h"

#include "../Modeling/CtStructureTree.h"
#include "StructureArtifact.h"

#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>


ArtifactStructureWrapper::ArtifactStructureWrapper(CtStructure& structure) :
        Structure(&structure) {
}

void ArtifactStructureWrapper::AddStructureArtifact(StructureArtifact& structureArtifact) {
    StructureArtifacts.emplace_back(&structureArtifact);

    std::sort(StructureArtifacts.begin(), StructureArtifacts.end(),
              [](const auto& a, const auto b) {
        return a && b && a->GetArtifactSubType() < b->GetArtifactSubType();
    });
}

void ArtifactStructureWrapper::RemoveStructureArtifact(StructureArtifact& structureArtifact) {
    auto it = std::find(StructureArtifacts.begin(), StructureArtifacts.end(), &structureArtifact);
    if (it == StructureArtifacts.end()) {
        qWarning("Cannot remove structure artifact. Given artifact not contained in this list");
        return;
    }

    StructureArtifacts.erase(it);
}

ArtifactStructureWrapper::TypeToStructureArtifactsMap ArtifactStructureWrapper::GetStructureArtifactMap() {
    auto structureArtifactTypes = Artifact::GetStructureArtifactTypes();

    TypeToStructureArtifactsMap map;
    for (const auto& subType: structureArtifactTypes)
        map[subType] = std::vector<StructureArtifact*>();

    for (const auto& artifact: StructureArtifacts)
        map[artifact->GetArtifactSubType()].push_back(artifact);

    return map;
}

void ArtifactStructureWrapper::AddArtifactValuesAtPositionToMap(const double* x,
                                                                std::map<Artifact::SubType, float>& artifactValueMap) {
    for (const auto& artifact : StructureArtifacts)
        artifactValueMap[artifact->GetArtifactSubType()] += artifact->EvaluateAtPosition(x);
}

vtkMTimeType ArtifactStructureWrapper::GetMTime() const {
    if (StructureArtifacts.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(StructureArtifacts.size());
    std::transform(StructureArtifacts.begin(), StructureArtifacts.end(), std::back_inserter(artifactMTimes),
                   [](StructureArtifact* artifact) { return artifact->GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}

vtkStandardNewMacro(TreeStructureArtifactCollection)

vtkMTimeType TreeStructureArtifactCollection::GetMTime() {
    if (ArtifactLists.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(ArtifactLists.size());
    std::transform(ArtifactLists.begin(), ArtifactLists.end(), std::back_inserter(artifactMTimes),
                   [](const auto& wrapper) { return wrapper.GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}

void TreeStructureArtifactCollection::AddStructureArtifactList(CtStructure& ctStructure) {
    ArtifactLists.emplace_back(ctStructure);
}

void TreeStructureArtifactCollection::RemoveStructureArtifactList(CtStructure& structure) {
    auto removeIt = std::find_if(ArtifactLists.begin(), ArtifactLists.end(),
                              [&](const auto& wrapper) { return wrapper.Structure == &structure; });
    if (removeIt == ArtifactLists.end()) {
        vtkWarningMacro("Cannot remove structure artifact list. Given structure is not associated with a stored list");
        return;
    }

    ArtifactLists.erase(removeIt);
}
