#include "StructureArtifactsFilter.h"

#include "StructureWrapper.h"

#include <vtkDataSetAttributes.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTypeUInt16Array.h>

vtkStandardNewMacro(StructureArtifactsFilter)

void StructureArtifactsFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Structure Artifact Collection: [" << StructureArtifactCollection << ")\n";
}

vtkMTimeType StructureArtifactsFilter::GetMTime() {
    vtkMTimeType mTime = Superclass::GetMTime();
    vtkMTimeType treeMTime = StructureArtifactCollection ? StructureArtifactCollection->GetMTime() : 0;

    return std::max({ mTime, treeMTime });
}

void StructureArtifactsFilter::SetTreeStructureArtifactCollection(
        TreeStructureArtifactCollection* structureArtifactCollection) {
    vtkSetObjectBodyMacro(StructureArtifactCollection, TreeStructureArtifactCollection, structureArtifactCollection);
}

vtkExecutive* StructureArtifactsFilter::CreateDefaultExecutive() {
    return vtkStreamingDemandDrivenPipeline::New();
}

int StructureArtifactsFilter::RequestInformation(vtkInformation* vtkNotUsed(request),
                                                 vtkInformationVector** vtkNotUsed(inputVector),
                                                 vtkInformationVector *outputVector) {

    vtkInformation* outInfo = outputVector->GetInformationObject(0);

//    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), GetWholeExtent().data(), 6);
//    outInfo->Set(vtkDataObject::ORIGIN(), GetOrigin().data(), 3);
//    outInfo->Set(vtkDataObject::SPACING(), GetSpacing().data(), 3);

    outInfo->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    vtkDataObject::SetActiveAttributeInfo(outInfo,
                                          vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS,
                                          "Radiodensity",
                                          VTK_FLOAT, 1, -1);

    return 1;
}

void StructureArtifactsFilter::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) {
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

    Algorithm sampleAlgorithm(this, data, radiodensities, functionValues, basicStructureIds);

    vtkSMPTools::For(0, numberOfPoints, sampleAlgorithm);
}

StructureArtifactsFilter::Algorithm::Algorithm(StructureArtifactsFilter* self,
                                               vtkImageData* volumeData,
                                               float* radiodensities,
                                               float* functionValues,
                                               uint16_t* basicStructureIds) :
        Self(self),
        VolumeData(volumeData),
        Radiodensities(radiodensities),
        FunctionValues(functionValues),
        BasicStructureIds(basicStructureIds) {

}

void StructureArtifactsFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    double point[3];
    Self->CheckAbort();

    while (pointId < endPointId) {

        if (Self->GetAbortOutput()) {
            return;
        }

        VolumeData->GetPoint(pointId, point);

//        CtStructure::ModelingResult result = Tree->FunctionValueAndRadiodensity(point);
//
//        bool pointIsWithinStructure = result.FunctionValue < 0;
//        FunctionValues[pointId] = result.FunctionValue;
//        Radiodensities[pointId] = pointIsWithinStructure
//                                  ? result.Radiodensity
//                                  : -1000.0f;
//        BasicStructureIds[pointId] = pointIsWithinStructure
//                                     ? result.BasicCtStructureId
//                                     : 0;

        pointId++;
    }
}
