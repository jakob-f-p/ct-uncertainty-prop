#pragma once

#include <vtkObject.h>

class CtDataCsgTree;
class ImageArtifactConcatenation;

class Pipeline : public vtkObject {
public:
    static Pipeline* New();
    vtkTypeMacro(Pipeline, vtkObject);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    std::string GetName() const;

    void SetCtDataTree(CtDataCsgTree* ctDataCsgTree);
    vtkGetObjectMacro(CtDataTree, CtDataCsgTree);

    void InitializeWithAppDataTree();

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
