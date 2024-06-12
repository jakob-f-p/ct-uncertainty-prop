#include "Pipeline.h"

#include "Image/ImageArtifactConcatenation.h"
#include "Structure/StructureArtifactListCollection.h"
#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"

Pipeline::Pipeline(CtStructureTree& structureTree, CtDataSource& dataSource, std::string name) :
        Name(name.empty()
                ? "Pipeline " + std::to_string(PipelineId++)
                : std::move(name)),
        StructureTree(structureTree),
        DataSource(dataSource),
        TreeStructureArtifacts(new TreeStructureArtifactListCollection(structureTree)),
        ImageArtifactConcat(new ImageArtifactConcatenation()) {

    for (uidx_t i = 0; i < structureTree.StructureCount(); ++i)
        TreeStructureArtifacts->AddStructureArtifactList(i);
};

Pipeline::~Pipeline() = default;

auto Pipeline::GetName() const noexcept -> std::string {
    return Name;
}

auto Pipeline::GetMTime() const noexcept -> vtkMTimeType {
    ImageArtifactConcat->UpdateArtifactFilter();
    return std::max({ TreeStructureArtifacts->GetMTime(), ImageArtifactConcat->GetFilterMTime(); });
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

auto Pipeline::GetImageAlgorithm() const -> vtkImageAlgorithm& {
    auto& treeArtifactsFilter = TreeStructureArtifacts->GetFilter();
    treeArtifactsFilter.SetInputConnection(DataSource.GetOutputPort());

    ImageArtifactConcat->UpdateArtifactFilter();
    auto& imageArtifactStartFilter = ImageArtifactConcat->GetStartFilter();
    imageArtifactStartFilter.SetInputConnection(treeArtifactsFilter.GetOutputPort());

    return ImageArtifactConcat->GetEndFilter();
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

uint16_t Pipeline::PipelineId = 1;
