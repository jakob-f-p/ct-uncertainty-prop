#include "Pipeline.h"

#include "ImageArtifactConcatenation.h"
#include "StructureArtifactListCollection.h"
#include "../Modeling/CtStructureTree.h"

Pipeline::Pipeline(uidx_t structureCount) :
        TreeStructureArtifacts(new TreeStructureArtifactListCollection()),
        ImageArtifactConcat(new ImageArtifactConcatenation()) {

    for (uidx_t i = 0; i < structureCount; ++i)
        TreeStructureArtifacts->AddStructureArtifactList(i);
};

Pipeline::~Pipeline() = default;

auto Pipeline::GetName() const noexcept -> std::string {
    return Name;
}

auto Pipeline::GetStructureArtifactListCollectionForIdx(uidx_t structureIdx) const -> StructureArtifactList& {
    return TreeStructureArtifacts->GetForCtStructureIdx(structureIdx);
}

auto Pipeline::GetImageArtifactConcatenation() const -> ImageArtifactConcatenation& {
    return *ImageArtifactConcat;
}

void Pipeline::ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) {
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
