#pragma once

#include "StructureWrapper.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>

class CtStructureTree;
class ImageArtifactConcatenation;

struct CtStructureTreeEvent;

class Pipeline : public vtkObject {
public:
    Pipeline(const Pipeline&) = delete;
    void operator=(const Pipeline&) = delete;

    static Pipeline* New();
    vtkTypeMacro(Pipeline, vtkObject);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    std::string GetName() const;

    void SetCtDataTree(CtStructureTree* ctStructureTree);
    [[nodiscard]] CtStructureTree* GetCtDataTree() const;

    ImageArtifactConcatenation& GetImageArtifactConcatenation();

    void ProcessCtStructureTreeEvent(CtStructureTreeEvent event);

protected:
    Pipeline() = default;
    ~Pipeline() override = default;

    std::string Name;
    vtkSmartPointer<CtStructureTree> CtDataTree;
    vtkNew<TreeStructureArtifactCollection> TreeStructureArtifacts;
    vtkNew<ImageArtifactConcatenation> ImageArtifactConcatenation;
};
