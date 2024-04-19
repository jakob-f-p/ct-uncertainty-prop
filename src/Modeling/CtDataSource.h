#pragma once

#include <vtkImageAlgorithm.h>

#include <array>

class vtkImageData;

class CtStructureTree;

class CtDataSource : public vtkImageAlgorithm {
public:
    static CtDataSource* New();
    vtkTypeMacro(CtDataSource, vtkImageAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void SetDataTree(CtStructureTree* ctStructureTree);

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
    ~CtDataSource() override = default;

    vtkExecutive* CreateDefaultExecutive() override;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

    std::array<double, 3> GetSpacing();
    std::array<double, 3> GetOrigin();
    std::array<int, 6> GetWholeExtent();

    struct SampleAlgorithm {
        CtDataSource* Self;
        vtkImageData* VolumeData;
        CtStructureTree* Tree;
        float* Radiodensities;
        float* FunctionValues;
        uint16_t* BasicStructureIds;

        SampleAlgorithm(CtDataSource* self,
                        vtkImageData* volumeData,
                        CtStructureTree* tree,
                        float* radiodensities,
                        float* functionValues,
                        uint16_t* basicStructureIds);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    std::array<float, 3> PhysicalDimensions;
    std::array<int, 3> NumberOfVoxels;
    CtStructureTree* DataTree = nullptr;
};
