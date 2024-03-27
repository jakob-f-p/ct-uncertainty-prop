#include "Pipeline.h"

#include "ImageArtifactConcatenation.h"
#include "../App.h"
#include "../Modeling/CtDataCsgTree.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(Pipeline)

void Pipeline::PrintSelf(ostream &os, vtkIndent indent) {
    vtkObject::PrintSelf(os, indent);
}

std::string Pipeline::GetName() const {
    return Name;
}

void Pipeline::SetCtDataTree(CtDataCsgTree* ctDataCsgTree) {
    vtkSetObjectBodyMacro(CtDataTree, CtDataCsgTree, ctDataCsgTree)
}


void Pipeline::InitializeWithAppDataTree() {
    auto* newDataTree = CtDataCsgTree::New();
    newDataTree->DeepCopy(&App::GetInstance()->GetCtDataCsgTree());
    SetCtDataTree(newDataTree);

    newDataTree->FastDelete();
}

Pipeline::Pipeline() :
        CtDataTree(nullptr),
        ImageArtifactConcatenation(*ImageArtifactConcatenation::New()) {
}

Pipeline::~Pipeline() {
    if (CtDataTree) {
        CtDataTree->Delete();
    }

    ImageArtifactConcatenation.Delete();
}

ImageArtifactConcatenation& Pipeline::GetImageArtifactConcatenation() {
    return ImageArtifactConcatenation;
}
