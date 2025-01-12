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

    CopyInputArrayAttributesToOutput(request, inputVector, outputVector);

    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    outInfo->CopyEntry(inInfo, vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
    outInfo->CopyEntry(inInfo, vtkDataObject::ORIGIN());
    outInfo->CopyEntry(inInfo, vtkDataObject::SPACING());
    outInfo->CopyEntry(inInfo, vtkDataObject::POINT_DATA_VECTOR());

    outInfo->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    return 1;
}

auto ImageArtifactFilter::RequestData(vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector) -> int {
    const int outputPort = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());

    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation* outInfo = outputVector->GetInformationObject(outputPort);
    vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    output->SetExtent(input->GetExtent());

    output->GetPointData()->PassData(input->GetPointData());

    static_cast<void>(GetDeepCopiedRadiodensitiesArray(input, output));

    SetErrorCode(vtkErrorCode::NoError);

    ExecuteDataWithImageInformation(input, output, outInfo);

    if (GetErrorCode() != 0U)
        return 0;

    return 1;
}

auto ImageArtifactFilter::AddArrayInformationToPointDataVector(SubType subType,
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

auto ImageArtifactFilter::GetArrayName(SubType subType) noexcept -> std::string {
    return SubTypeToString(subType);
}

auto ImageArtifactFilter::PointDataInformationVectorHasArray(vtkInformation* info, SubType subType) -> bool {
    vtkInformationVector* pointDataVector = info->Get(vtkDataObject::POINT_DATA_VECTOR());
    for (int i = 0; i < pointDataVector->GetNumberOfInformationObjects(); ++i) {
        if (vtkInformation* pointDataInfo = pointDataVector->GetInformationObject(i);
            pointDataInfo->Has(vtkDataObject::FIELD_NAME()) != 0) {
            if (auto pointDataArrayName = std::string(pointDataInfo->Get(vtkDataObject::FIELD_NAME()));
                pointDataArrayName == GetArrayName(subType))
                return true;
        }
    }

    return false;
}

auto ImageArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
                                                          vtkImageData* output,
                                                          vtkInformation* outInfo) -> void {
    throw std::runtime_error("not implemented");
}

auto ImageArtifactFilter::GetRadiodensitiesArray(vtkImageData* imageData) noexcept -> vtkFloatArray* {
    return vtkFloatArray::SafeDownCast(imageData->GetPointData()->GetAbstractArray("Radiodensities"));
}

auto ImageArtifactFilter::GetDeepCopiedRadiodensitiesArray(vtkImageData* input, vtkImageData* output) noexcept
        -> vtkFloatArray* {

    vtkNew<vtkFloatArray> copiedRadiodensities;
    copiedRadiodensities->DeepCopy(GetRadiodensitiesArray(input));

    output->GetPointData()->AddArray(copiedRadiodensities);

    return copiedRadiodensities;
}

auto ImageArtifactFilter::GetArtifactArray(vtkImageData* imageData, SubType subType) noexcept -> vtkFloatArray* {
    vtkFloatArray* dataArray = vtkFloatArray::SafeDownCast(
            imageData->GetPointData()->GetAbstractArray(GetArrayName(subType).data()));

    if (dataArray)
        return dataArray;

    return AddArtifactArray(imageData, subType);
}

auto ImageArtifactFilter::GetDeepCopiedArtifactArray(vtkImageData* input,
                                                     vtkImageData* output,
                                                     SubType subType) noexcept -> vtkFloatArray* {
    vtkFloatArray* artifactArray = vtkFloatArray::SafeDownCast(
            input->GetPointData()->GetAbstractArray(GetArrayName(subType).data()));

    if (artifactArray) {
        vtkNew<vtkFloatArray> copiedArtifactArray;
        copiedArtifactArray->DeepCopy(artifactArray);

        output->GetPointData()->AddArray(copiedArtifactArray);

        return copiedArtifactArray;
    }

    return AddArtifactArray(output, subType);
}

void ImageArtifactFilter::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) {
    throw std::runtime_error("not used by image artifact filter");
}

auto ImageArtifactFilter::AddArtifactArray(vtkImageData* imageData,
                                           SubType subType) noexcept -> vtkFloatArray* {
    vtkNew<vtkFloatArray> dataArray;
    dataArray->SetNumberOfComponents(1);
    dataArray->SetName(GetArrayName(subType).data());
    dataArray->SetNumberOfTuples(imageData->GetNumberOfPoints());
    dataArray->FillValue(0.0F);
    imageData->GetPointData()->AddArray(dataArray);

    return dataArray;
}
