#include "CtDataSource.h"

#include "CtStructureTree.h"

#include <vtkDataSetAttributes.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTypeUInt16Array.h>

vtkStandardNewMacro(CtDataSource)

void CtDataSource::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Physical Dimensions: [" << PhysicalDimensions[0] << ", " << PhysicalDimensions[1] << ", " << PhysicalDimensions[2] << "]\n";
    os << indent << "Number of Voxels: [" << NumberOfVoxels[0] << ", " << NumberOfVoxels[1] << ", " << NumberOfVoxels[2] << "]\n";
    os << indent << "Implicit CT Structures: (" << DataTree << ")\n";
}

vtkMTimeType CtDataSource::GetMTime() {
    vtkMTimeType mTime = Superclass::GetMTime();
    vtkMTimeType treeMTime = DataTree ? DataTree->GetMTime() : 0;

    return std::max({ mTime, treeMTime });
}

void CtDataSource::SetDataTree(CtStructureTree* ctStructureTree) {
    vtkSetObjectBodyMacro(DataTree, CtStructureTree, ctStructureTree)
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

    int defaultResolution = 128; //256;
    std::fill(NumberOfVoxels.begin(), NumberOfVoxels.end(), defaultResolution);

    float defaultPhysicalDimensionsLength = 100.0f;
    std::fill(PhysicalDimensions.begin(), PhysicalDimensions.end(), defaultPhysicalDimensionsLength);

    CtDataSource::SetNumberOfInputPorts(0);
}

vtkExecutive* CtDataSource::CreateDefaultExecutive() {
    return vtkStreamingDemandDrivenPipeline::New();
}

int CtDataSource::RequestInformation(vtkInformation* vtkNotUsed(request),
                                     vtkInformationVector** vtkNotUsed(inputVector),
                                     vtkInformationVector *outputVector) {

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
    ZoneScopedN("ExecuteDataWithInformation");

    vtkImageData* data = vtkImageData::SafeDownCast(output);
    int* updateExtent = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    data->SetExtent(updateExtent);
    vtkIdType numberOfPoints = data->GetNumberOfPoints();

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

    SampleAlgorithm sampleAlgorithm(this, data, DataTree, radiodensities, functionValues, basicStructureIds);

    vtkSMPTools::For(0, numberOfPoints, sampleAlgorithm);
}

std::array<double, 3> CtDataSource::GetSpacing() {
    std::array<double, 3> spacing {};

    for (int i = 0; i < spacing.size(); ++i) {
        spacing[i] = PhysicalDimensions[i] / static_cast<double>(NumberOfVoxels[i]);
    }

    return spacing;
}

std::array<double, 3> CtDataSource::GetOrigin() {
    std::array<double, 3> origin {};

    for (int i = 0; i < origin.size(); ++i) {
        origin[i] = -PhysicalDimensions[i] / 2.0;
    }

    return origin;
}

std::array<int, 6> CtDataSource::GetWholeExtent() {
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
        Tree(tree),
        Radiodensities(radiodensities),
        FunctionValues(functionValues),
        BasicStructureIds(basicStructureIds) {

}

void CtDataSource::SampleAlgorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    ZoneScopedN("SampleAlgorithm");
    double point[3];
    Self->CheckAbort();

    while (pointId < endPointId) {

        if (Self->GetAbortOutput()) {
            return;
        }

        VolumeData->GetPoint(pointId, point);

        CtStructure::ModelingResult result = Tree->FunctionValueAndRadiodensity(point);

        bool pointIsWithinStructure = result.FunctionValue < 0;
        FunctionValues[pointId] = result.FunctionValue;
        Radiodensities[pointId] = pointIsWithinStructure
                                    ? result.Radiodensity
                                    : -1000.0f;
        BasicStructureIds[pointId] = pointIsWithinStructure
                                            ? result.BasicCtStructureId
                                            : 0;

        pointId++;
    }
}
