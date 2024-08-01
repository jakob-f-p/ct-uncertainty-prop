#include "CtDataSource.h"

#include <vtkDataSetAttributes.h>
#include <vtkDataObject.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

void CtDataSource::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Physical Dimensions: [" << PhysicalDimensions[0] << ", "
                                             << PhysicalDimensions[1] << ", "
                                             << PhysicalDimensions[2] << "]\n";
    os << indent << "Number of Voxels: [" << NumberOfVoxels[0] << ", "
                                          << NumberOfVoxels[1] << ", "
                                          << NumberOfVoxels[2] << "]\n";
}

CtDataSource::CtDataSource() {
#ifdef BUILD_TYPE_DEBUG
//    int const defaultResolution = 8;
    int const defaultResolution = 16;
#else
//    int const defaultResolution = 256;
    int const defaultResolution = 192;
#endif
    std::fill(NumberOfVoxels.begin(), NumberOfVoxels.end(), defaultResolution);

    float const defaultPhysicalDimensionsLength = 100.0F;
    std::fill(PhysicalDimensions.begin(), PhysicalDimensions.end(), defaultPhysicalDimensionsLength);

    CtDataSource::SetNumberOfInputPorts(0);
}

auto CtDataSource::CreateDefaultExecutive() -> vtkExecutive* {
    return vtkStreamingDemandDrivenPipeline::New();
}

auto CtDataSource::RequestInformation(vtkInformation* vtkNotUsed(request),
                                      vtkInformationVector** vtkNotUsed(inputVector),
                                      vtkInformationVector *outputVector) -> int {

    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), GetWholeExtent().data(), 6);
    outInfo->Set(vtkDataObject::ORIGIN(), GetOrigin().data(), 3);
    outInfo->Set(vtkDataObject::SPACING(), GetSpacing().data(), 3);

    outInfo->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    vtkDataObject::SetActiveAttributeInfo(outInfo,
                                          vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS,
                                          "Radiodensity",
                                          VTK_FLOAT, 1, -1);

    return 1;
}

auto CtDataSource::GetSpacing() -> std::array<double, 3> {
    std::array<double, 3> spacing {};

    for (int i = 0; i < spacing.size(); ++i) {
        spacing[i] = PhysicalDimensions[i] / static_cast<double>(NumberOfVoxels[i]);
    }

    return spacing;
}

auto CtDataSource::GetOrigin() -> std::array<double, 3> {
    std::array<double, 3> origin {};

    for (int i = 0; i < origin.size(); ++i) {
        origin[i] = -PhysicalDimensions[i] / 2.0;
    }

    return origin;
}

auto CtDataSource::GetWholeExtent() -> std::array<int, 6> {
    std::array<int, 6> extent {};
    std::fill(extent.begin(), extent.end(), 0);

    for (int i = 0; i < NumberOfVoxels.size(); ++i) {
        extent[2 * i + 1] = NumberOfVoxels[i];
    }

    return extent;
}

auto CtDataSource::GetDimensions() -> std::array<int, 3> {
    auto const extent = GetWholeExtent();

    std::array<int, 3> dims {};
    for (int i = 0; i < 3; ++i)
        dims[i] = extent[2 * i + 1] - extent[2 * i] + 1;

    return dims;
}
