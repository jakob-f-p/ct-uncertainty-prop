#include "StairStepArtifactFilter.h"
#include "../../../Modeling/CtDataSource.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>

#include <numbers>

vtkStandardNewMacro(StairStepArtifactFilter)

void StairStepArtifactFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Sampling rate: " << SamplingRate << ")\n";
}

auto StairStepArtifactFilter::RequestInformation(vtkInformation* request,
                                                vtkInformationVector** inputVector,
                                                vtkInformationVector *outputVector) -> int {
    ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);

    AddArrayInformationToPointDataVector(SubType::STAIR_STEP, outputVector);

    return 1;
}

void StairStepArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
                                                              vtkImageData* output,
                                                              vtkInformation* outInfo) {
    vtkIdType const numberOfPoints = output->GetNumberOfPoints();

    vtkNew<vtkFloatArray> const newArtifactValueArray;
    newArtifactValueArray->SetNumberOfComponents(1);
    newArtifactValueArray->SetNumberOfTuples(numberOfPoints);
    newArtifactValueArray->FillValue(0.0F);
    float* newArtifactValues = newArtifactValueArray->WritePointer(0, numberOfPoints);

    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);
    float* radiodensities = radioDensityArray->WritePointer(0, numberOfPoints);

    std::array<double, 6> bounds {};
    output->GetBounds(bounds.data());
    double const zDistance = bounds[5] - bounds[4];

    std::array<int, 6> previousExtent {}, newExtent {};
    output->GetExtent(previousExtent.data());
    newExtent = previousExtent;
    int const zPreviousNrOfVoxels = previousExtent[5] - previousExtent[4];
    int const zNewNrOfVoxels = std::max(static_cast<int>(zPreviousNrOfVoxels * SamplingRate), 2);
    newExtent[5] = zNewNrOfVoxels;

    std::array<double, 3> previousOutputSpacing {}, newOutputSpacing {};
    output->GetSpacing(previousOutputSpacing.data());
    newOutputSpacing = previousOutputSpacing;
    newOutputSpacing[2] = zDistance / static_cast<double>(zNewNrOfVoxels);

    vtkNew<vtkImageReslice> const imageDownSample;
    imageDownSample->SetOutputSpacing(newOutputSpacing.data());
    imageDownSample->SetOutputExtent(newExtent.data());
    imageDownSample->SetInputData(input);
    imageDownSample->Update();
    vtkSmartPointer const downSampledImage = imageDownSample->GetOutput();

    vtkNew<vtkImageReslice> const imageUpSample;
    imageUpSample->SetOutputSpacing(previousOutputSpacing.data());
    imageUpSample->SetOutputExtent(previousExtent.data());
    imageUpSample->SetInputData(downSampledImage);
    imageUpSample->Update();
    vtkSmartPointer const upSampledImage = imageUpSample->GetOutput();

    assert(output->GetNumberOfPoints() == upSampledImage->GetNumberOfPoints());

    vtkFloatArray* resampledRadiodensityArray = vtkFloatArray::SafeDownCast(upSampledImage->GetPointData()->GetScalars());
    float* resampledRadiodensities = resampledRadiodensityArray->WritePointer(0, numberOfPoints);

    auto calculateSamplingArtifactValues = [newArtifactValues, resampledRadiodensities, radiodensities] (vtkIdType pointId, vtkIdType endPointId) {
        for (; pointId < endPointId; pointId++)
            newArtifactValues[pointId] = resampledRadiodensities[pointId] - radiodensities[pointId];
    };

    vtkSMPTools::For(0, numberOfPoints, calculateSamplingArtifactValues);


    vtkFloatArray* stairStepArtifactArray = GetDeepCopiedArtifactArray(input, output, SubType::STAIR_STEP);
    float* stairStepArtifactValues = stairStepArtifactArray->WritePointer(0, numberOfPoints);

    auto addArtifactValues = [stairStepArtifactValues, newArtifactValues, radiodensities] (vtkIdType pointId, vtkIdType endPointId) {
        vtkIdType const startPointId = pointId;

        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += newArtifactValues[pointId];

        pointId = startPointId;
        for (; pointId < endPointId; pointId++)
            stairStepArtifactValues[pointId] += newArtifactValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addArtifactValues);
}
