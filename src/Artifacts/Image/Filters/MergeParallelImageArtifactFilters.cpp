#include "MergeParallelImageArtifactFilters.h"

#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(MergeParallelImageArtifactFilters)

MergeParallelImageArtifactFilters::MergeParallelImageArtifactFilters() {
    vtkAlgorithm::SetNumberOfInputPorts(2);
}

auto MergeParallelImageArtifactFilters::SetBaseFilterConnection(vtkAlgorithmOutput* input) noexcept -> void {
    SetInputConnection(0, input);
}

auto MergeParallelImageArtifactFilters::AddParallelFilterConnection(vtkAlgorithmOutput* input) noexcept -> void {
    AddInputConnection(1, input);
}

int MergeParallelImageArtifactFilters::FillInputPortInformation(int port, vtkInformation* info) {
    if (port < 0 || port > 1)
        return 0;

    ImageArtifactFilter::FillInputPortInformation(port, info);

    if (port == 1)
        info->Set(INPUT_IS_REPEATABLE(), 1);

    return 1;
}

int MergeParallelImageArtifactFilters::RequestInformation(vtkInformation* request,
                                                          vtkInformationVector** inputVector,
                                                          vtkInformationVector* outputVector) {
    vtkInformation* baseInInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformationVector* parallelInInfos = inputVector[1];

    CopyInputArrayAttributesToOutput(request, inputVector, outputVector);
    outInfo->CopyEntry(baseInInfo, vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
    outInfo->CopyEntry(baseInInfo, vtkDataObject::ORIGIN());
    outInfo->CopyEntry(baseInInfo, vtkDataObject::SPACING());
    outInfo->CopyEntry(baseInInfo, vtkDataObject::POINT_DATA_VECTOR());

    outInfo->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    for (std::vector<SubType> const containedSubTypes = GetContainedSubTypes(baseInInfo, parallelInInfos);
         const auto subType : containedSubTypes)
        AddArrayInformationToPointDataVector(subType, outputVector);

    return 1;
}

int MergeParallelImageArtifactFilters::RequestData(vtkInformation* request,
                                                   vtkInformationVector** inputVector,
                                                   vtkInformationVector* outputVector) {
    vtkInformation* baseInInfo = inputVector[0]->GetInformationObject(0);
    vtkInformationVector* parallelInInfos = inputVector[1];
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    vtkImageData* baseInput = vtkImageData::SafeDownCast(baseInInfo->Get(vtkDataObject::DATA_OBJECT()));
    std::vector<vtkImageData*> parallelInputs;
    for (int i = 0; i < parallelInInfos->GetNumberOfInformationObjects(); ++i) {
        vtkInformation* parallelInfo = parallelInInfos->GetInformationObject(i);
        parallelInputs.push_back(vtkImageData::SafeDownCast(parallelInfo->Get(vtkDataObject::DATA_OBJECT())));
    }
    vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    output->SetExtent(baseInput->GetExtent());
    vtkIdType const numberOfPoints = output->GetNumberOfPoints();

    vtkPointData* outputPointData = output->GetPointData();
    outputPointData->PassData(baseInput->GetPointData());

    std::vector<SubType> containedSubTypes = GetContainedSubTypes(baseInInfo, parallelInInfos);

    std::vector<vtkFloatArray*> outputArrays;
    std::ranges::transform(containedSubTypes, std::back_inserter(outputArrays),
                           [&, baseInput, output](SubType subType) { return GetDeepCopiedArtifactArray(baseInput, output, subType); });
    outputArrays.push_back(GetDeepCopiedRadiodensitiesArray(baseInput, output));

    std::vector<float*> artifactArrayWritePointers;
    std::ranges::transform(outputArrays, std::back_inserter(artifactArrayWritePointers),
                           [=](vtkFloatArray* array) { return array->WritePointer(0, numberOfPoints); });

    std::vector<std::vector<vtkFloatArray*>> inputArrays;
    std::ranges::transform(containedSubTypes, std::back_inserter(inputArrays),
                           [&, parallelInInfos](SubType subType) {
                               std::vector<vtkFloatArray*> subTypeInputArrays;

                               for (int i = 0; i < parallelInInfos->GetNumberOfInformationObjects(); i++) {
                                   if (vtkInformation* parallelInfo = parallelInInfos->GetInformationObject(i);
                                       PointDataInformationVectorHasArray(parallelInfo, subType))
                                       subTypeInputArrays.push_back(GetArtifactArray(parallelInputs[i], subType));
                               }

                               return subTypeInputArrays;
                           });
    std::vector<vtkFloatArray*> inputRadiodensityArrays;
    std::ranges::transform(parallelInputs, std::back_inserter(inputRadiodensityArrays),
                           [&](vtkImageData* parallelInput) { return GetRadiodensitiesArray(parallelInput); });
    inputArrays.push_back(inputRadiodensityArrays);


    std::vector<std::vector<const float*>> inputArrayReadPointers;
    std::ranges::transform(inputArrays, std::back_inserter(inputArrayReadPointers),
                           [=](std::vector<vtkFloatArray*> subTypeInputArrays) {
                               std::vector<const float*> subTypeReadPointers;
                               std::ranges::transform(subTypeInputArrays, std::back_inserter(subTypeReadPointers),
                                                      [=](vtkFloatArray* array) { return array->WritePointer(0, numberOfPoints); });
                               return subTypeReadPointers;
                           });


    auto addInputArrayDeltasToOutputArrays = [artifactArrayWritePointers, inputArrayReadPointers]
    (vtkIdType pointId, vtkIdType endPointId) {
        vtkIdType const startPointId = pointId;

        for (int i = 0; i < artifactArrayWritePointers.size(); ++i) {
            float* outArray = artifactArrayWritePointers[i];
            std::vector<float const*> const& inArrays = inputArrayReadPointers[i];

            std::vector<float> pointDeltas(endPointId - pointId, 0.0);

            for (auto const*const inArray: inArrays) {
                pointId = startPointId;

                for (int j = 0; pointId < endPointId; pointId++, j++)
                    pointDeltas[j] += inArray[pointId] - outArray[pointId];
            }
            pointId = startPointId;

            for (int j = 0; pointId < endPointId; pointId++, j++)
                outArray[pointId] += pointDeltas[j];

            pointId = startPointId;
        }
    };
    vtkSMPTools::For(0, numberOfPoints, addInputArrayDeltasToOutputArrays);


    SetErrorCode(vtkErrorCode::NoError);

    if (GetErrorCode() != 0U)
        return 0;

    return 1;
}

auto MergeParallelImageArtifactFilters::InfoPointDataInformationVectorHasArray(vtkInformationVector* infos,
                                                                               SubType subType) -> bool {
    for (int i = 0; i < infos->GetNumberOfInformationObjects(); ++i) {
        if (PointDataInformationVectorHasArray(infos->GetInformationObject(i), subType))
            return true;
    }

    return false;
}

auto MergeParallelImageArtifactFilters::GetContainedSubTypes(vtkInformation* baseInInfo,
                                                             vtkInformationVector* parallelInInfos) noexcept
                                                             -> std::vector<SubType> {
    std::vector<SubType> containedSubTypes;
    for (auto const& [name, enumValue] : BasicImageArtifactDetails::GetSubTypeValues()) {
        if (PointDataInformationVectorHasArray(baseInInfo, enumValue)
            || InfoPointDataInformationVectorHasArray(parallelInInfos, enumValue)) {
            containedSubTypes.push_back(enumValue);
            break;
        }
    }
    return containedSubTypes;
}
