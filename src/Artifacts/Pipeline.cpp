#include "Pipeline.h"

#include "ImageArtifactConcatenation.h"
#include "../App.h"
#include "../Modeling/CtStructureTree.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(Pipeline)

void Pipeline::PrintSelf(ostream &os, vtkIndent indent) {
    vtkObject::PrintSelf(os, indent);
}

std::string Pipeline::GetName() const {
    return Name;
}

void Pipeline::SetCtDataTree(CtStructureTree* ctStructureTree) {
    if (!ctStructureTree || CtDataTree == ctStructureTree)
        return;

    CtDataTree = ctStructureTree;

    Modified();
}


CtStructureTree* Pipeline::GetCtDataTree() const {
    return CtDataTree;
}

ImageArtifactConcatenation& Pipeline::GetImageArtifactConcatenation() {
    return *ImageArtifactConcatenation;
}

void Pipeline::ProcessCtStructureTreeEvent(CtStructureTreeEvent event) {
    switch (event.Type) {
        case CtStructureTreeEventType::Add: {
            TreeStructureArtifacts->AddStructureArtifactList(event.Structure);
            break;
        }

        case CtStructureTreeEventType::Remove: {
            TreeStructureArtifacts->RemoveStructureArtifactList(event.Structure);
            break;
        }
    }
}
