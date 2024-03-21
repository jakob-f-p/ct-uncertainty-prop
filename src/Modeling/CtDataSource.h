#pragma once

#include "CtDataCsgTree.h"

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>

#include <tracy/Tracy.hpp>

class CtDataSource : public vtkImageAlgorithm {
public:
    static CtDataSource* New();
    vtkTypeMacro(CtDataSource, vtkImageAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    vtkSetObjectMacro(DataTree, CtDataCsgTree);

    /**
     * Set physical dimensions of the scanned image in mm along each axis.
     */
    void SetVolumeDataPhysicalDimensions(float x, float y, float z);

    /**
     * Set number of voxels along each axis.
     */
    void SetVolumeNumberOfVoxels(int x, int y, int z);

    CtDataSource(const CtDataSource&) = delete;
    void operator=(const CtDataSource&) = delete;

protected:
    CtDataSource();
    ~CtDataSource() override;

    vtkExecutive *CreateDefaultExecutive() override;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

    std::array<double, 3> GetSpacing();
    std::array<double, 3> GetOrigin();
    std::array<int, 6> GetWholeExtent();

    class SampleAlgorithm {
    public:
        CtDataSource* Self;
        vtkImageData* VolumeData;
        CtDataCsgTree* Tree;
        float* Radiodensities;
        float* FunctionValues;
        uint16_t* ImplicitCtStructureIds;

        SampleAlgorithm(CtDataSource* self,
                        vtkImageData* volumeData,
                        CtDataCsgTree* tree,
                        float* radiodensities,
                        float* functionValues,
                        uint16_t* implicitCtStructureIds);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    std::array<float, 3> PhysicalDimensions;
    std::array<int, 3> NumberOfVoxels;
    CtDataCsgTree* DataTree;
};
