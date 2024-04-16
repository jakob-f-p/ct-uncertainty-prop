#include "Pipeline.h"
#include "../Modeling/CtStructureTree.h"

Pipeline::Pipeline(StructureIdx structureCount) :
        TreeStructureArtifacts(new TreeStructureArtifactCollection()),
        ImageArtifactConcat(new ImageArtifactConcatenation()) {

    for (StructureIdx i = 0; i < structureCount; ++i)
        TreeStructureArtifacts->AddStructureArtifactList(i);
};

auto Pipeline::GetName() const noexcept -> std::string {
    return Name;
}

auto Pipeline::GetArtifactStructureWrapper(StructureIdx structureIdx) const -> StructureArtifacts& {
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
    }
}

auto Pipeline::operator==(const Pipeline& other) const noexcept -> bool {
    return Name == other.Name
            && &TreeStructureArtifacts == &other.TreeStructureArtifacts
            && &ImageArtifactConcat == &other.ImageArtifactConcat;
}
