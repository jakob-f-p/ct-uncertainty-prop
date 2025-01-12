#pragma once

#include "../Utils/LinearAlgebraTypes.h"

#include <vtkImageAlgorithm.h>


class CtDataSource : public vtkImageAlgorithm {
public:
    vtkAbstractTypeMacro(CtDataSource, vtkImageAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    /**
     * Set physical dimensions of the scanned image in mm along each axis.
     */
    void SetVolumeDataPhysicalDimensions(FloatVector dimensions);

    FloatVector GetVolumeDataPhysicalDimensions() const noexcept;

    /**
     * Set number of voxels along each axis.
     */
    void SetVolumeNumberOfVoxels(std::array<int, 3> voxelCounts);

    std::array<int, 3> GetVolumeNumberOfVoxels() const noexcept;

    CtDataSource(const CtDataSource&) = delete;
    void operator=(const CtDataSource&) = delete;

protected:
    CtDataSource();
    ~CtDataSource() override = default;

    vtkExecutive* CreateDefaultExecutive() override;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    std::array<double, 3> GetSpacing() const;

    std::array<double, 3> GetOrigin() const;

    std::array<int, 6> GetWholeExtent() const;

    std::array<int, 3> GetDimensions() const;


    FloatVector PhysicalDimensions {};
    std::array<int, 3> NumberOfVoxels {};
};
