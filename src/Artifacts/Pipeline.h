#pragma once

#include <vtkObject.h>

class CtStructureTree;
class ImageArtifactConcatenation;

class Pipeline : public vtkObject {
public:
    static Pipeline* New();
    vtkTypeMacro(Pipeline, vtkObject);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    std::string GetName() const;

    void SetCtDataTree(CtStructureTree* ctStructureTree);
    vtkGetObjectMacro(CtDataTree, CtStructureTree);

    void InitializeWithAppDataTree();

    ImageArtifactConcatenation& GetImageArtifactConcatenation();

    Pipeline(const Pipeline&) = delete;
    void operator=(const Pipeline&) = delete;

protected:
    Pipeline();
    ~Pipeline() override;

    CtStructureTree* CtDataTree;
    ImageArtifactConcatenation& ImageArtifactConcatenation;
    std::string Name;
};
