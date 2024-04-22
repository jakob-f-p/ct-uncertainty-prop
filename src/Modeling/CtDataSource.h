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

    [[nodiscard]] auto static
    PointIdToDimensionCoordinates(vtkIdType pointId,
                                  const std::array<int, 3> dimensions) -> std::array<int, 3> {
        const int dimX = dimensions[0];
        const int dimX_dimY = dimensions[0] * dimensions[1];
        const int z = pointId / dimX_dimY;
        const int y = (pointId - z * dimX_dimY) / dimX;
        const int x = pointId - z * dimX_dimY - y * dimX;
        return { x, y, z };
    };

    [[nodiscard]] auto static
    GetDecrementedCoordinates(const std::array<int, 3> coordinates,
                              const std::array<int, 3> dimensions) -> std::array<int, 3> {
        std::array<int, 3> decrementedCoordinates = coordinates;

        if (coordinates[0] == 0) {
            decrementedCoordinates[0] = dimensions[0] - 1;

            if (coordinates[1] == 0) {
                decrementedCoordinates[1] = dimensions[1] - 1;

                decrementedCoordinates[2] = coordinates[2] - 1;
            } else {
                decrementedCoordinates[1] = coordinates[1] - 1;
            }
        } else {
            decrementedCoordinates[0] = coordinates[0] - 1;
        }

        return decrementedCoordinates;
    };

    struct SampleAlgorithm {
        CtDataSource* Self;
        vtkImageData* VolumeData;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
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
