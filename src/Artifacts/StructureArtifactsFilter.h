#pragma once

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

class TreeStructureArtifactCollection;

class StructureArtifactsFilter : public vtkImageAlgorithm {
public:
    StructureArtifactsFilter(const StructureArtifactsFilter&) = delete;
    void operator=(const StructureArtifactsFilter&) = delete;
    static StructureArtifactsFilter* New();
    vtkTypeMacro(StructureArtifactsFilter, vtkImageAlgorithm)

    struct Algorithm {
        StructureArtifactsFilter* Self;
        vtkImageData* VolumeData;
//        CtStructureTree* Tree;
        float* Radiodensities;
        float* FunctionValues;
        uint16_t* BasicStructureIds;

        Algorithm(StructureArtifactsFilter* self,
                  vtkImageData* volumeData,
//                  CtStructureTree* tree,
                  float* radiodensities,
                  float* functionValues,
                  uint16_t* basicStructureIds);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };;

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void SetTreeStructureArtifactCollection(TreeStructureArtifactCollection* structureArtifactCollection);

protected:
    StructureArtifactsFilter() = default;
    ~StructureArtifactsFilter() override = default;

    vtkExecutive* CreateDefaultExecutive() override;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

    vtkSmartPointer<TreeStructureArtifactCollection> StructureArtifactCollection;
};
