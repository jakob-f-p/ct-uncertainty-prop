#include "MergeParallelImageArtifactFilters.h"

#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>

vtkStandardNewMacro(MergeParallelImageArtifactFilters)

MergeParallelImageArtifactFilters::MergeParallelImageArtifactFilters() {
    SetNumberOfInputPorts(2);
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
        info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);

    return 1;
}

int MergeParallelImageArtifactFilters::RequestInformation(vtkInformation* request,
                                                          vtkInformationVector** inputVector,
                                                          vtkInformationVector* outputVector) {
    vtkInformation* baseInInfo = inputVector[0]->GetInformationObject(0);
    vtkInformationVector* parallelInInfos = inputVector[1];
    CopyInputArrayAttributesToOutput(request, inputVector, outputVector);

    std::vector<SubType> containedSubTypes = GetContainedSubTypes(baseInInfo, parallelInInfos);

    for (const auto subType : containedSubTypes)
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
    vtkIdType numberOfPoints = output->GetNumberOfPoints();


    vtkPointData* outputPointData = output->GetPointData();
    outputPointData->PassData(baseInput->GetPointData());

    std::vector<SubType> containedSubTypes = GetContainedSubTypes(baseInInfo, parallelInInfos);

    std::vector<vtkFloatArray*> outputArrays;
    std::transform(containedSubTypes.begin(), containedSubTypes.end(), std::back_inserter(outputArrays),
                   [&, output](SubType subType) { return GetArtifactArray(output, subType); });

    std::vector<float*> artifactArrayWritePointers;
    std::transform(outputArrays.begin(), outputArrays.end(), std::back_inserter(artifactArrayWritePointers),
                   [=](vtkFloatArray* array) { return array->WritePointer(0, numberOfPoints); });

    std::vector<std::vector<vtkFloatArray*>> inputArrays;
    std::transform(containedSubTypes.begin(), containedSubTypes.end(), std::back_inserter(inputArrays),
                   [&, parallelInInfos](SubType subType) {
        std::vector<vtkFloatArray*> subTypeInputArrays;

        for (int i = 0; i < parallelInInfos->GetNumberOfInformationObjects(); i++) {
            vtkInformation* parallelInfo = parallelInInfos->GetInformationObject(i);

            if (PointDataInformationVectorHasArray(parallelInfo, subType))
                subTypeInputArrays.push_back(GetArtifactArray(parallelInputs[i], subType));
        }

        return subTypeInputArrays;
    });

    std::vector<std::vector<const float*>> inputArrayReadPointers;
    std::transform(inputArrays.begin(), inputArrays.end(), std::back_inserter(inputArrayReadPointers),
                   [=](std::vector<vtkFloatArray*> subTypeInputArrays) {
        std::vector<const float*> subTypeReadPointers;
        std::transform(subTypeInputArrays.begin(), subTypeInputArrays.end(), std::back_inserter(subTypeReadPointers),
                       [=](vtkFloatArray* array) { return array->WritePointer(0, numberOfPoints); });
        return subTypeReadPointers;
    });


    auto addInputArrayDeltasToOutputArrays = [containedSubTypes, artifactArrayWritePointers, inputArrayReadPointers]
    (vtkIdType pointId, vtkIdType endPointId) {
        for (int i = 0; i < containedSubTypes.size(); ++i) {
            float* outArray = artifactArrayWritePointers[i];
            std::vector<const float*> inArrays = inputArrayReadPointers[i];

            std::vector<float> pointDeltas(endPointId - pointId, 0.0);

            for (const auto inArray: inArrays) {
                for (int j = 0; pointId < endPointId; pointId++, j++)
                    pointDeltas[j] += inArray[pointId] - outArray[pointId];
            }

            for (int j = 0; pointId < endPointId; pointId++, j++)
                outArray[pointId] += pointDeltas[j];
        }
    };
    vtkSMPTools::For(0, numberOfPoints, addInputArrayDeltasToOutputArrays);


    SetErrorCode(vtkErrorCode::NoError);

    if (GetErrorCode())
        return 0;

    return 1;
}

auto MergeParallelImageArtifactFilters::InfoPointDataInformationVectorHasArray(vtkInformationVector* infos,
                                                                               SubType subType) -> bool {
    for (int i = 0; i < infos->GetNumberOfInformationObjects(); ++i) {
        if (ImageArtifactFilter::PointDataInformationVectorHasArray(infos->GetInformationObject(i), subType))
            return true;
    }

    return false;
}

auto MergeParallelImageArtifactFilters::GetContainedSubTypes(vtkInformation* baseInInfo,
                                                             vtkInformationVector* parallelInInfos) noexcept
                                                             -> std::vector<SubType> {
    std::vector<SubType> containedSubTypes;
    for (auto subTypeAndName : BasicImageArtifactDetails::GetSubTypeValues()) {
        if (PointDataInformationVectorHasArray(baseInInfo, subTypeAndName.EnumValue)
            || InfoPointDataInformationVectorHasArray(parallelInInfos, subTypeAndName.EnumValue)) {
            containedSubTypes.push_back(subTypeAndName.EnumValue);
            break;
        }
    }
    return containedSubTypes;
}
