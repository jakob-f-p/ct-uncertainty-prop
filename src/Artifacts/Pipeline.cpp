#include "Pipeline.h"

#include "Image/ImageArtifactConcatenation.h"
#include "Structure/StructureArtifactListCollection.h"
#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"
#include "../PipelineGroups/ArtifactVariantPointer.h"

Pipeline::Pipeline(CtStructureTree& structureTree, std::string name) :
        Name(name.empty()
                ? "Pipeline " + std::to_string(PipelineId++)
                : std::move(name)),
        StructureTree(structureTree),
        TreeStructureArtifacts(new TreeStructureArtifactListCollection(structureTree,
                                                                       [this](StructureArtifact& artifact) {
            BeforeArtifactRemoved(ArtifactVariantPointer { &artifact });
        })),
        ImageArtifactConcat(new ImageArtifactConcatenation([this](ImageArtifact& artifact) {
            BeforeArtifactRemoved(ArtifactVariantPointer { &artifact });
        })) {

    for (uidx_t i = 0; i < structureTree.StructureCount(); ++i)
        TreeStructureArtifacts->AddStructureArtifactList(i);
};

Pipeline::~Pipeline() = default;

auto Pipeline::GetName() const noexcept -> std::string {
    return Name;
}

auto Pipeline::GetMTime() const noexcept -> vtkMTimeType {
    ImageArtifactConcat->UpdateArtifactFilter();
    return std::max({ TreeStructureArtifacts->GetMTime(), ImageArtifactConcat->GetFilterMTime() });
}

auto Pipeline::GetCtStructureTree() const noexcept -> CtStructureTree& {
    return StructureTree;
}

auto Pipeline::GetStructureArtifactList(uint16_t structureIdx) const -> StructureArtifactList& {
    return TreeStructureArtifacts->GetForCtStructureIdx(structureIdx);
}

auto Pipeline::GetStructureArtifactListCollection() const -> TreeStructureArtifactListCollection& {
    return *TreeStructureArtifacts;
}

auto Pipeline::GetImageArtifactConcatenation() const -> ImageArtifactConcatenation& {
    return *ImageArtifactConcat;
}

auto Pipeline::GetArtifactsAlgorithm() const -> AlgorithmPipeline {
    ImageArtifactConcat->UpdateArtifactFilter();
    auto& treeArtifactsFilter = TreeStructureArtifacts->GetFilter();
    auto& imageArtifactStartFilter = ImageArtifactConcat->GetStartFilter();
    imageArtifactStartFilter.SetInputConnection(treeArtifactsFilter.GetOutputPort());

    return { treeArtifactsFilter, ImageArtifactConcat->GetEndFilter() };
}

auto Pipeline::GetImageArtifactsAlgorithm() const -> AlgorithmPipeline {
    ImageArtifactConcat->UpdateArtifactFilter();
    return { ImageArtifactConcat->GetStartFilter(), ImageArtifactConcat->GetEndFilter() };
}

void Pipeline::ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) const {
    switch (event.Type) {
        case CtStructureTreeEventType::ADD: {
            TreeStructureArtifacts->AddStructureArtifactList(event.Idx);
            break;
        }

        case CtStructureTreeEventType::REMOVE: {
            TreeStructureArtifacts->RemoveStructureArtifactList(event.Idx);
            break;
        }

        case CtStructureTreeEventType::EDIT: {}
    }
}

auto Pipeline::operator==(const Pipeline& other) const noexcept -> bool {
    return Name == other.Name
            && &TreeStructureArtifacts == &other.TreeStructureArtifacts
            && &ImageArtifactConcat == &other.ImageArtifactConcat;
}

auto
Pipeline::AddBeforeArtifactRemovedCallback(void* receiver,
                                           Pipeline::BeforeArtifactRemovedCallback&& callback) const noexcept -> void {
    CallbackMap.emplace(receiver, std::move(callback));
}

auto Pipeline::RemoveBeforeArtifactRemovedCallback(void* receiver) const noexcept -> void {
    CallbackMap.erase(receiver);
}

auto Pipeline::BeforeArtifactRemoved(ArtifactVariantPointer const& artifactVariantPointer) const -> void {
    for (auto const& [receiver, callback] : CallbackMap)
        callback(artifactVariantPointer);
}

uint16_t Pipeline::PipelineId = 1;
