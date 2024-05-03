#include "StructureArtifactListCollection.h"

#include "StructureArtifact.h"

auto StructureArtifactList::GetMTime() const noexcept -> vtkMTimeType {
    if (Artifacts.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(Artifacts.size());
    std::transform(Artifacts.begin(), Artifacts.end(), std::back_inserter(artifactMTimes),
                   [](auto& artifact) { return artifact->GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}


auto StructureArtifactList::Get(int idx) const -> StructureArtifact& {
    if (idx < 0 || idx >= Artifacts.size())
        throw std::runtime_error("Structure artifact index out of range");

    return *Artifacts.at(idx);
}

auto StructureArtifactList::GetNumberOfArtifacts() const noexcept -> uidx_t {
    return static_cast<int>(Artifacts.size());
}

void StructureArtifactList::AddStructureArtifact(StructureArtifact& structureArtifact, int insertionIdx) {
    if (insertionIdx == -1)
        Artifacts.emplace_back(std::make_unique<StructureArtifact>(std::move(structureArtifact)));
    else
        Artifacts.emplace(std::next(Artifacts.begin(), insertionIdx), std::make_unique<StructureArtifact>(std::move(structureArtifact)));

    std::sort(Artifacts.begin(), Artifacts.end(),
              [](const auto& a, const auto& b) {
        return a && b && a->GetSubType() < b->GetSubType();
    });
}

void StructureArtifactList::RemoveStructureArtifact(StructureArtifact& structureArtifact) {
    auto it = std::find_if(Artifacts.begin(), Artifacts.end(),
                           [&](auto& artifact) { return artifact.get() == &structureArtifact; });

    if (it == Artifacts.end())
        std::runtime_error("Cannot remove structure artifact. Given artifact not contained in this list");

    Artifacts.erase(it);
}

//StructureArtifactList::TypeToStructureArtifactsMap StructureArtifactList::GetStructureArtifactMap() {
//    auto structureArtifactTypes = Artifact::GetStructureArtifactTypes();
//
//    TypeToStructureArtifactsMap map;
//    for (const auto& subType: structureArtifactTypes)
//        map[subType] = std::vector<StructureArtifact*>();
//
//    for (const auto& artifact: StructureArtifactList)
//        map[artifact->GetArtifactSubType()].push_back(artifact);
//
//    return map;
//}

void StructureArtifactList::MoveStructureArtifact(StructureArtifact& artifact, int newIdx) {
    if (newIdx < 0 || newIdx >= Artifacts.size())
        throw std::runtime_error("Cannot move given image artifact to index");

    auto previousIt = std::find_if(Artifacts.begin(), Artifacts.end(),
                                   [&](auto& structureArtifact) { return structureArtifact.get() == &artifact; });
    if (previousIt == Artifacts.end()) {
        qWarning("Cannot move given image artifact to index");
        return;
    }
    auto currentIdx = std::distance(Artifacts.begin(), previousIt);

    if (currentIdx == newIdx)
        return;

    auto newIt = std::next(Artifacts.begin(), newIdx);
    if (currentIdx < newIdx)
        std::rotate(previousIt, std::next(previousIt), std::next(newIt));
    else
        std::rotate(newIt, std::next(newIt), std::next(previousIt));
}

StructureArtifactList::~StructureArtifactList() = default;

//void StructureArtifactList::AddArtifactValuesAtPositionToMap(const double* x,
//                                                          std::map<Artifact::SubType, float>& artifactValueMap) {
//    for (const auto& artifact : StructureArtifactList)
//        artifactValueMap[artifact->GetArtifactSubType()] += artifact->EvaluateAtPosition(x);
//}


auto TreeStructureArtifactListCollection::GetMTime() -> vtkMTimeType {
    if (ArtifactLists.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(ArtifactLists.size());
    std::transform(ArtifactLists.begin(), ArtifactLists.end(), std::back_inserter(artifactMTimes),
                   [](const auto& wrapper) { return wrapper.GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}

auto TreeStructureArtifactListCollection::GetForCtStructureIdx(uidx_t structureIdx) -> StructureArtifactList& {
    if (structureIdx >= ArtifactLists.size())
        throw std::runtime_error("No artifact structure wrapper for the given structure exists within this collection");

    return ArtifactLists[structureIdx];
}

void TreeStructureArtifactListCollection::AddStructureArtifactList(uidx_t insertionIdx) {
    if (insertionIdx > ArtifactLists.size())
        throw std::runtime_error("Cannot add structure wrapper. Given index is  larger then the current size.");

    ArtifactLists.emplace_back();
}

void TreeStructureArtifactListCollection::RemoveStructureArtifactList(uidx_t removeIdx) {
    if (removeIdx >= ArtifactLists.size())
        throw std::runtime_error("Cannot remove structure artifact list. No structure exists at given index");

    ArtifactLists.erase(std::next(ArtifactLists.begin(), removeIdx));
}
