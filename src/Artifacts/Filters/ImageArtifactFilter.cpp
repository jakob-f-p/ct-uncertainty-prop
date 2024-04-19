#include "ImageArtifactFilter.h"

#include <vtkErrorCode.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkFloatArray.h>

auto ImageArtifactFilter::CreateDefaultExecutive() -> vtkExecutive* {
    return vtkStreamingDemandDrivenPipeline::New();
}

auto ImageArtifactFilter::RequestInformation(vtkInformation* request,
                                             vtkInformationVector** inputVector,
                                             vtkInformationVector* outputVector) -> int {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    outInfo->Copy(inInfo);

    outInfo->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    return 1;
}

int ImageArtifactFilter::RequestData(vtkInformation* request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector) {
    const int outputPort = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());

    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation* outInfo = outputVector->GetInformationObject(outputPort);
    vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    output->GetPointData()->PassData(input->GetPointData());

    SetErrorCode(vtkErrorCode::NoError);

    ExecuteDataWithImageInformation(output, outInfo);

    if (GetErrorCode())
        return 0;

    return 1;
}

auto ImageArtifactFilter::AddArrayInformationToPointDataVector(Artifact::SubType subType,
                                                               vtkInformationVector* outputVector) -> void {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    if (PointDataInformationVectorHasArray(outInfo, subType))
        return;

    vtkInformation* arrayDataInformation = vtkInformation::New();
    arrayDataInformation->Set(vtkDataObject::FIELD_ASSOCIATION(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
    arrayDataInformation->Set(vtkDataObject::FIELD_NAME(), GetArrayName(subType));
    arrayDataInformation->Set(vtkDataObject::FIELD_ARRAY_TYPE(), VTK_FLOAT);
    arrayDataInformation->Set(vtkDataObject::FIELD_NUMBER_OF_COMPONENTS(), 1);

    vtkInformationVector* pointDataVector = outInfo->Get(vtkDataObject::POINT_DATA_VECTOR());
    pointDataVector->Append(arrayDataInformation);
    arrayDataInformation->FastDelete();
}

auto ImageArtifactFilter::GetArrayName(Artifact::SubType subType) noexcept -> std::string {
    return Artifact::SubTypeToString(subType);
}

auto ImageArtifactFilter::PointDataInformationVectorHasArray(vtkInformation* info,
                                                             Artifact::SubType subType) -> bool {
    vtkInformationVector* pointDataVector = info->Get(vtkDataObject::POINT_DATA_VECTOR());
    for (int i = 0; i < pointDataVector->GetNumberOfInformationObjects(); ++i) {
        vtkInformation* pointDataInfo = pointDataVector->GetInformationObject(i);

        if (pointDataInfo->Has(vtkDataObject::FIELD_NAME()) != 0) {
            auto pointDataArrayName = std::string(pointDataInfo->Get(vtkDataObject::FIELD_NAME()));

            if (pointDataArrayName == GetArrayName(subType))
                return true;
        }
    }

    return false;
}

auto ImageArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* output, vtkInformation* outInfo) -> void {
    throw std::runtime_error("not implemented");
}

auto ImageArtifactFilter::GetRadiodensitiesArray(vtkImageData* output) noexcept -> vtkFloatArray* {
    return vtkFloatArray::SafeDownCast(output->GetPointData()->GetAbstractArray("Radiodensities"));
}

auto ImageArtifactFilter::GetArtifactArray(vtkImageData* output, Artifact::SubType subType) noexcept -> vtkFloatArray* {
    vtkFloatArray* dataArray = vtkFloatArray::SafeDownCast(
            output->GetPointData()->GetAbstractArray(GetArrayName(subType).data()));

    if (dataArray)
        return dataArray;

    dataArray = vtkFloatArray::New();
    dataArray->SetNumberOfComponents(1);
    dataArray->SetName(GetArrayName(subType).data());
    dataArray->SetNumberOfTuples(output->GetNumberOfPoints());
    dataArray->FillValue(0.0F);
    output->GetPointData()->AddArray(dataArray);
    dataArray->Delete();

    return dataArray;
}

void ImageArtifactFilter::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) {
    throw std::runtime_error("not used by image artifact filter");
}
