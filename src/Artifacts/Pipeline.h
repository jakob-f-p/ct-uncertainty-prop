#pragma once

#include <vtkObject.h>
#include <vtkSmartPointer.h>

class ArtifactStructureWrapper;
class CtStructure;
class CtStructureTree;
class ImageArtifactConcatenation;
class TreeStructureArtifactCollection;

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

    [[nodiscard]] ArtifactStructureWrapper& GetArtifactStructureWrapper(const CtStructure& structure) const;

    [[nodiscard]] ImageArtifactConcatenation& GetImageArtifactConcatenation();

    void ProcessCtStructureTreeEvent(CtStructureTreeEvent event);

protected:
    Pipeline() = default;
    ~Pipeline() override = default;

    std::string Name;
    vtkSmartPointer<CtStructureTree> CtDataTree;
    vtkNew<TreeStructureArtifactCollection> TreeStructureArtifacts;
    vtkNew<ImageArtifactConcatenation> ImageArtifactConcatenation;
};
