#include "StructureArtifactListCollection.h"

#include "StructureArtifactsFilter.h"
#include "../../Modeling/CtStructureTree.h"

auto StructureArtifactList::GetMTime() const noexcept -> vtkMTimeType {
    if (Artifacts.empty())
        return 0;

    std::vector<vtkMTimeType> artifactMTimes(Artifacts.size());
    std::transform(Artifacts.begin(), Artifacts.end(), std::back_inserter(artifactMTimes),
                   [](auto& artifact) { return artifact.GetMTime(); });
    return *std::max_element(artifactMTimes.begin(), artifactMTimes.end());
}


auto StructureArtifactList::Contains(StructureArtifact const& structureArtifact) const noexcept -> bool {
    auto it = std::find(Artifacts.cbegin(), Artifacts.cend(), structureArtifact);

    return it != Artifacts.cend();
}

auto StructureArtifactList::Get(int idx) -> StructureArtifact& {
    if (idx < 0 || idx >= Artifacts.size())
        throw std::runtime_error("Structure artifact index out of range");

    return Artifacts.at(idx);
}

auto StructureArtifactList::GetNumberOfArtifacts() const noexcept -> uidx_t {
    return static_cast<int>(Artifacts.size());
}

void StructureArtifactList::AddStructureArtifact(StructureArtifact&& structureArtifact, int insertionIdx) {
    if (insertionIdx == -1)
        insertionIdx = Artifacts.size();

    Artifacts.emplace(std::next(Artifacts.begin(), insertionIdx), std::move(structureArtifact));

//    auto insertIt = Artifacts.emplace(std::next(Artifacts.begin(), insertionIdx), std::move(structureArtifact));
//    auto const mTime = insertIt->GetMTime();

    std::sort(Artifacts.begin(), Artifacts.end(),
              [](auto const& a, auto const& b) { return a.GetSubType() < b.GetSubType(); });
//
//    auto findIt = std::find_if(Artifacts.begin(), Artifacts.end(),
//                               [mTime](auto const& artifact) { return artifact.GetMTime() == mTime; });
//    if (findIt == Artifacts.end())
//        throw std::runtime_error("inserted artifact not found");
//
//    return *structureArtifact;
}

void StructureArtifactList::RemoveStructureArtifact(StructureArtifact const& structureArtifact) {
    auto it = std::find(Artifacts.begin(), Artifacts.end(), structureArtifact);

    if (it == Artifacts.end())
        throw std::runtime_error("Cannot remove structure artifact. Given artifact not contained in this list");

    BeforeRemoveCallback(*it);

    Artifacts.erase(it);
}

void StructureArtifactList::MoveStructureArtifact(StructureArtifact const& artifact, int newIdx) {
    if (newIdx < 0 || newIdx >= Artifacts.size())
        throw std::runtime_error("Cannot move given image artifact to index");

    auto previousIt = std::find(Artifacts.begin(), Artifacts.end(), artifact);
    if (previousIt == Artifacts.end())
        throw std::runtime_error("Cannot move given image artifact to index");

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



TreeStructureArtifactListCollection::TreeStructureArtifactListCollection(
        CtStructureTree const& ctStructureTree,
        BeforeRemoveArtifactCallback&& removeCallback) :

        BeforeRemoveCallback(removeCallback),
        StructureTree(ctStructureTree) {
    Filter->SetStructureArtifactCollection(this);
};

TreeStructureArtifactListCollection::~TreeStructureArtifactListCollection() = default;

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

    auto basicStructureIdProvider = [&](StructureArtifactList const& artifactList) {
        auto it = std::find(ArtifactLists.cbegin(), ArtifactLists.cend(), artifactList);
        if (it == ArtifactLists.cend())
            throw std::runtime_error("Artifact list must exist");

        uidx_t const artifactListIdx = std::distance(ArtifactLists.cbegin(), it);

        return StructureTree.GetBasicStructureIdsOfStructureAt(artifactListIdx);
    };

    auto structureEvaluatorProvider = [&](StructureArtifactList const& artifactList) {
        auto it = std::find(ArtifactLists.cbegin(), ArtifactLists.cend(), artifactList);
        if (it == ArtifactLists.cend())
            throw std::runtime_error("Artifact list must exist");

        uidx_t const artifactListIdx = std::distance(ArtifactLists.cbegin(), it);

        auto const& structure = StructureTree.GetStructureAt(artifactListIdx);

        return [&](DoublePoint point) { return StructureTree.FunctionValue(point, structure); };
    };

    auto tissueValueProvider = [&](StructureArtifactList const& artifactList) {
        auto it = std::find(ArtifactLists.cbegin(), ArtifactLists.cend(), artifactList);
        if (it == ArtifactLists.cend())
            throw std::runtime_error("Artifact list must exist");

        uidx_t const artifactListIdx = std::distance(ArtifactLists.cbegin(), it);

        auto const& structure = StructureTree.GetStructureAt(artifactListIdx);

        return [&](DoublePoint point) {
            return StructureTree.FunctionValueAndRadiodensity(point, &structure).Radiodensity;
        };
    };

    ArtifactLists.emplace(std::next(ArtifactLists.cbegin(), insertionIdx),
                          BeforeRemoveCallback, basicStructureIdProvider,
                          tissueValueProvider, structureEvaluatorProvider);
}

void TreeStructureArtifactListCollection::RemoveStructureArtifactList(uidx_t removeIdx) {
    if (removeIdx >= ArtifactLists.size())
        throw std::runtime_error("Cannot remove structure artifact list. No structure exists at given index");

    ArtifactLists.erase(std::next(ArtifactLists.begin(), removeIdx));
}

auto TreeStructureArtifactListCollection::GetFilter() const -> vtkImageAlgorithm& {
    return *Filter;
}

auto TreeStructureArtifactListCollection::GetStructureArtifactList(StructureArtifact const& structureArtifact) const
        -> StructureArtifactList const& {

    auto it = std::find_if(ArtifactLists.begin(), ArtifactLists.end(),
                           [&structureArtifact](auto const& list) { return list.Contains(structureArtifact); });

    if (it == ArtifactLists.cend())
        throw std::runtime_error("Could not find the list for given structure artifact");

    return *it;
}

auto TreeStructureArtifactListCollection::GetIdx(StructureArtifactList const& structureArtifactList) const -> uidx_t {
    auto it = std::find_if(ArtifactLists.cbegin(), ArtifactLists.cend(),
                           [&structureArtifactList](auto const& list) { return &list == &structureArtifactList; });

    if (it == ArtifactLists.cend())
        throw std::runtime_error("Could not find given list");

    return std::distance(ArtifactLists.cbegin(), it);
}
