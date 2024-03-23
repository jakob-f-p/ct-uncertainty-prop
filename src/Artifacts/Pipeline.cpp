#include "Pipeline.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(Pipeline)

void Pipeline::PrintSelf(ostream &os, vtkIndent indent) {
    vtkObject::PrintSelf(os, indent);
}

std::string Pipeline::GetName() const {
    return Name;
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
