#include "Pipeline.h"

#include "Image/ImageArtifactConcatenation.h"
#include "Structure/StructureArtifactListCollection.h"
#include "../Modeling/CtStructureTree.h"

Pipeline::Pipeline(CtStructureTree const& structureTree) :
        TreeStructureArtifacts(new TreeStructureArtifactListCollection(structureTree)),
        ImageArtifactConcat(new ImageArtifactConcatenation()) {

    for (uidx_t i = 0; i < structureTree.StructureCount(); ++i)
        TreeStructureArtifacts->AddStructureArtifactList(i);
};

Pipeline::~Pipeline() = default;

auto Pipeline::GetName() const noexcept -> std::string {
    return Name;
}

auto Pipeline::GetStructureArtifactListCollectionForIdx(uidx_t structureIdx) const -> StructureArtifactList& {
    return TreeStructureArtifacts->GetForCtStructureIdx(structureIdx);
}

auto Pipeline::GetTreeArtifacts() const -> TreeStructureArtifactListCollection& {
    return *TreeStructureArtifacts;
}

auto Pipeline::GetImageArtifactConcatenation() const -> ImageArtifactConcatenation& {
    return *ImageArtifactConcat;
}

void Pipeline::ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) const {
    switch (event.Type) {
        case CtStructureTreeEventType::Add: {
            TreeStructureArtifacts->AddStructureArtifactList(event.Idx);
            break;
        }

        case CtStructureTreeEventType::Remove: {
            TreeStructureArtifacts->RemoveStructureArtifactList(event.Idx);
            break;
        }

        case CtStructureTreeEventType::Edit: {}
    }
}

auto Pipeline::operator==(const Pipeline& other) const noexcept -> bool {
    return Name == other.Name
            && &TreeStructureArtifacts == &other.TreeStructureArtifacts
            && &ImageArtifactConcat == &other.ImageArtifactConcat;
}
