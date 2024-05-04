#include "CtDataSource.h"

#include "CtStructureTree.h"
#include "../ImageDataUtils.h"

#include <vtkDataSetAttributes.h>
#include <vtkDataObject.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTypeUInt16Array.h>

vtkStandardNewMacro(CtDataSource)

void CtDataSource::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Physical Dimensions: [" << PhysicalDimensions[0] << ", "
                                             << PhysicalDimensions[1] << ", "
                                             << PhysicalDimensions[2] << "]\n";
    os << indent << "Number of Voxels: [" << NumberOfVoxels[0] << ", "
                                          << NumberOfVoxels[1] << ", "
                                          << NumberOfVoxels[2] << "]\n";
    os << indent << "Implicit CT Structures: (" << DataTree << ")\n";
}

auto CtDataSource::GetMTime() -> vtkMTimeType {
    vtkMTimeType const mTime = Superclass::GetMTime();
    vtkMTimeType const treeMTime = DataTree ? DataTree->GetMTime() : 0;

    return std::max({ mTime, treeMTime });
}

void CtDataSource::SetDataTree(CtStructureTree* ctStructureTree) {
    DataTree = ctStructureTree;
}

void CtDataSource::SetVolumeDataPhysicalDimensions(float x, float y, float z) {
    PhysicalDimensions = { x, y, z };
}

void CtDataSource::SetVolumeNumberOfVoxels(int x, int y, int z) {
    NumberOfVoxels = { x, y, z };
}

CtDataSource::CtDataSource() :
        PhysicalDimensions{},
        NumberOfVoxels{} {
#ifdef BUILD_TYPE_DEBUG
    int const defaultResolution = 64;
#else
    int const defaultResolution = 256;
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

void CtDataSource::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) {
    vtkImageData* data = vtkImageData::SafeDownCast(output);
    int* updateExtent = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    data->SetExtent(updateExtent);
    vtkIdType const numberOfPoints = data->GetNumberOfPoints();

    vtkFloatArray* radiodensityArray = vtkFloatArray::New();
    radiodensityArray->SetNumberOfComponents(1);
    radiodensityArray->SetName("Radiodensities");
    radiodensityArray->SetNumberOfTuples(numberOfPoints);
    data->GetPointData()->SetScalars(radiodensityArray);
    radiodensityArray->Delete();
    float* radiodensities = radiodensityArray->WritePointer(0, numberOfPoints);

    vtkFloatArray* functionValueArray = vtkFloatArray::New();
    functionValueArray->SetNumberOfComponents(1);
    functionValueArray->SetName("FunctionValues");
    functionValueArray->SetNumberOfTuples(numberOfPoints);
    data->GetPointData()->AddArray(functionValueArray);
    functionValueArray->Delete();
    float* functionValues = functionValueArray->WritePointer(0, numberOfPoints);

    vtkTypeUInt16Array* basicStructureIdArray= vtkTypeUInt16Array::New();
    basicStructureIdArray->SetNumberOfComponents(1);
    basicStructureIdArray->SetName("BasicStructureIds");
    basicStructureIdArray->SetNumberOfTuples(numberOfPoints);
    data->GetPointData()->AddArray(basicStructureIdArray);
    basicStructureIdArray->Delete();
    uint16_t* basicStructureIds = basicStructureIdArray->WritePointer(0, numberOfPoints);

    if (!DataTree->HasRoot()) {
        vtkDebugMacro("Ct data tree has 0 nodes. Cannot evaluate");
        return;
    }

    SampleAlgorithm sampleAlgorithm(this, data, DataTree, radiodensities, functionValues, basicStructureIds);
    vtkSMPTools::For(0, numberOfPoints, sampleAlgorithm);
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

CtDataSource::SampleAlgorithm::SampleAlgorithm(CtDataSource* self,
                                               vtkImageData* volumeData,
                                               CtStructureTree* tree,
                                               float* radiodensities,
                                               float* functionValues,
                                               uint16_t* basicStructureIds) :
        Self(self),
        VolumeData(volumeData),
        Spacing(self->GetSpacing()),
        Tree(tree),
        Radiodensities(radiodensities),
        FunctionValues(functionValues),
        BasicStructureIds(basicStructureIds) {
    std::copy(volumeData->GetDimensions(), std::next(volumeData->GetDimensions(), 3), UpdateDims.begin());
}

void CtDataSource::SampleAlgorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    Point startPoint;
    VolumeData->GetPoint(pointId, startPoint.data());

    const std::array<int, 3> startPointCoordinates = PointIdToDimensionCoordinates(pointId, UpdateDims);
    const std::array<int, 3> endPointCoordinates = PointIdToDimensionCoordinates(endPointId, UpdateDims);
    auto [ x1, y1, z1 ] = startPointCoordinates;
    auto lastValidPointCoordinates = GetDecrementedCoordinates(endPointCoordinates, UpdateDims);
    auto [ x2, y2, z2 ] = lastValidPointCoordinates;

    Point point = startPoint;
    for (vtkIdType z = z1; z <= z2; z++) {
        vtkIdType y = 0;
        if (z == z1)
            y = y1;

        vtkIdType yEnd = UpdateDims[1] - 1;
        if (z == z2 - 1)
            yEnd = y2;

        for (; y <= yEnd; y++) {
            vtkIdType x = 0;
            if (y == y1)
                x = x1;

            vtkIdType xEnd = UpdateDims[0] - 1;
            if (y == y2 - 1)
                xEnd = x2;

            for (; x <= xEnd; x++) {
                const CtStructureTree::ModelingResult result = Tree->FunctionValueAndRadiodensity(point);

                bool pointIsWithinStructure = result.FunctionValue < 0;
                FunctionValues[pointId] = result.FunctionValue;
                Radiodensities[pointId] = pointIsWithinStructure
                                          ? result.Radiodensity
                                          : -1000.0f;
                BasicStructureIds[pointId] = pointIsWithinStructure
                                             ? result.BasicCtStructureId
                                             : 0;
                pointId++;

                point[0] += Spacing[0];
            }
            point[0] = startPoint[0];
            point[1] += Spacing[1];
        }
        point[0] = startPoint[0];
        point[1] = startPoint[1];
        point[2] += Spacing[2];
    }
}
