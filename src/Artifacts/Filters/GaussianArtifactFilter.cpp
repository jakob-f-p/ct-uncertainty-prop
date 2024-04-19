#include "GaussianArtifactFilter.h"

#include <vtkBoxMuellerRandomSequence.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(GaussianArtifactFilter)

void GaussianArtifactFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Mean: " << Mean << "]\n";
    os << indent << "Standard Deviation: " << Sd << "]\n";
}

auto GaussianArtifactFilter::RequestInformation(vtkInformation* request,
                                                vtkInformationVector** inputVector,
                                                vtkInformationVector *outputVector) -> int {
    ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);

    AddArrayInformationToPointDataVector(Artifact::SubType::IMAGE_GAUSSIAN, outputVector);

    return 1;
}

void GaussianArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* output, vtkInformation* outInfo) {
    vtkIdType numberOfPoints = output->GetNumberOfPoints();

    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);

    vtkFloatArray* gaussianNoiseArray = GetArtifactArray(output, Artifact::SubType::IMAGE_GAUSSIAN);
    float* gaussianNoiseValues = gaussianNoiseArray->WritePointer(0, numberOfPoints);

    auto generateGaussianNoiseValues = [mean = Mean,
            sd = Sd,
            noiseValues = gaussianNoiseValues] (vtkIdType pointId, vtkIdType endPointId) {
        vtkNew<vtkBoxMuellerRandomSequence> gaussianSequence;

        for (; pointId < endPointId; pointId++)
            noiseValues[pointId] = static_cast<float>(gaussianSequence->GetNextScaledValue(mean, sd));
    };
    vtkSMPTools::For(0, numberOfPoints, generateGaussianNoiseValues);

    auto addNoiseValues = [noiseValues = gaussianNoiseValues,
            radiodensities = radioDensityArray->WritePointer(0, numberOfPoints)] (vtkIdType pointId,
                                                                                  vtkIdType endPointId) {
        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += noiseValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addNoiseValues);
}
