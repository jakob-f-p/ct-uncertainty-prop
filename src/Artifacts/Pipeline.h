#pragma once

#include "ImageArtifactConcatenation.h"
#include "../Modeling/CtDataCsgTree.h"

#include <vtkObject.h>

class Pipeline : public vtkObject {
public:
    static Pipeline* New();
    vtkTypeMacro(Pipeline, vtkObject);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    std::string GetName() const;

    vtkSetObjectMacro(CtDataTree, CtDataCsgTree);
    vtkGetObjectMacro(CtDataTree, CtDataCsgTree);

    ImageArtifactConcatenation& GetImageArtifactConcatenation();

    Pipeline(const Pipeline&) = delete;
    void operator=(const Pipeline&) = delete;

protected:
    Pipeline();
    ~Pipeline() override;

    CtDataCsgTree* CtDataTree;
    ImageArtifactConcatenation& ImageArtifactConcatenation;
    std::string Name;
};
