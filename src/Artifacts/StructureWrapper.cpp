#include "StructureWrapper.h"

#include "StructureArtifact.h"
#include "../Modeling/CtStructureTree.h"

auto StructureArtifacts::GetMTime() const noexcept -> vtkMTimeType {
    if (StructureArtifacts.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(StructureArtifacts.size());
    std::transform(StructureArtifacts.begin(), StructureArtifacts.end(), std::back_inserter(artifactMTimes),
                   [](StructureArtifact* artifact) { return artifact->GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}


auto StructureArtifacts::Get(int idx) const -> StructureArtifact& {
    if (idx < 0 || idx >= StructureArtifacts.size())
        throw std::runtime_error("Structure artifact index out of range");

    return *StructureArtifacts.at(idx);
}

auto StructureArtifacts::GetNumberOfArtifacts() const noexcept -> uidx_t {
    return static_cast<int>(StructureArtifacts.size());
}

void StructureArtifacts::AddStructureArtifact(StructureArtifact& structureArtifact, int insertionIdx) {
    if (insertionIdx == -1)
        StructureArtifacts.emplace_back(&structureArtifact);
    else
        StructureArtifacts.emplace(std::next(StructureArtifacts.begin(), insertionIdx), &structureArtifact);

    std::sort(StructureArtifacts.begin(), StructureArtifacts.end(),
              [](const auto& a, const auto b) {
        return a && b && a->GetArtifactSubType() < b->GetArtifactSubType();
    });
}

void StructureArtifacts::RemoveStructureArtifact(StructureArtifact& structureArtifact) {
    auto it = std::find(StructureArtifacts.begin(), StructureArtifacts.end(), &structureArtifact);
    if (it == StructureArtifacts.end()) {
        qWarning("Cannot remove structure artifact. Given artifact not contained in this list");
        return;
    }

    StructureArtifacts.erase(it);
}

StructureArtifacts::TypeToStructureArtifactsMap StructureArtifacts::GetStructureArtifactMap() {
    auto structureArtifactTypes = Artifact::GetStructureArtifactTypes();

    TypeToStructureArtifactsMap map;
    for (const auto& subType: structureArtifactTypes)
        map[subType] = std::vector<StructureArtifact*>();

    for (const auto& artifact: StructureArtifacts)
        map[artifact->GetArtifactSubType()].push_back(artifact);

    return map;
}

void StructureArtifacts::MoveStructureArtifact(StructureArtifact* artifact, int newIdx) {
    if (!artifact || newIdx < 0 || newIdx >= StructureArtifacts.size()) {
        qWarning("Cannot move given image artifact to index");
        return;
    }

    auto previousIt = std::find(StructureArtifacts.begin(), StructureArtifacts.end(), artifact);
    if (previousIt == StructureArtifacts.end()) {
        qWarning("Cannot move given image artifact to index");
        return;
    }
    auto currentIdx = std::distance(StructureArtifacts.begin(), previousIt);

    if (currentIdx == newIdx)
        return;

    auto newIt = std::next(StructureArtifacts.begin(), newIdx);
    if (currentIdx < newIdx)
        std::rotate(previousIt, std::next(previousIt), std::next(newIt));
    else
        std::rotate(newIt, std::next(newIt), std::next(previousIt));
}

void StructureArtifacts::AddArtifactValuesAtPositionToMap(const double* x,
                                                          std::map<Artifact::SubType, float>& artifactValueMap) {
    for (const auto& artifact : StructureArtifacts)
        artifactValueMap[artifact->GetArtifactSubType()] += artifact->EvaluateAtPosition(x);
}


auto TreeStructureArtifactCollection::GetMTime() -> vtkMTimeType {
    if (ArtifactLists.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(ArtifactLists.size());
    std::transform(ArtifactLists.begin(), ArtifactLists.end(), std::back_inserter(artifactMTimes),
                   [](const auto& wrapper) { return wrapper.GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}

StructureArtifacts& TreeStructureArtifactCollection::GetForCtStructureIdx(uidx_t structureIdx) {
    if (structureIdx >= ArtifactLists.size())
        throw std::runtime_error("No artifact structure wrapper for the given structure exists within this collection");

    return ArtifactLists[structureIdx];
}

void TreeStructureArtifactCollection::AddStructureArtifactList(uidx_t insertionIdx) {
    if (insertionIdx > ArtifactLists.size())
        throw std::runtime_error("Cannot add structure wrapper. Given index is  larger then the current size.");

    ArtifactLists.emplace_back();
}

void TreeStructureArtifactCollection::RemoveStructureArtifactList(uidx_t removeIdx) {
    if (removeIdx >= ArtifactLists.size())
        throw std::runtime_error("Cannot remove structure artifact list. No structure exists at given index");

    ArtifactLists.erase(std::next(ArtifactLists.begin(), removeIdx));
}
