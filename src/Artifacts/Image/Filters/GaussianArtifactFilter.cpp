#include "GaussianArtifactFilter.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <random>
#include <span>

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

    AddArrayInformationToPointDataVector(SubType::GAUSSIAN, outputVector);

    return 1;
}

void GaussianArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
                                                             vtkImageData* output,
                                                             vtkInformation* outInfo) {
    vtkIdType const numberOfPoints = output->GetNumberOfPoints();

    vtkNew<vtkFloatArray> newNoiseValueArray;
    newNoiseValueArray->SetNumberOfComponents(1);
    newNoiseValueArray->SetNumberOfTuples(output->GetNumberOfPoints());
    newNoiseValueArray->FillValue(0.0F);
    float* newNoiseValues = newNoiseValueArray->WritePointer(0, numberOfPoints);
    std::span<float> const newNoiseValueSpan { newNoiseValues, static_cast<size_t>(numberOfPoints) };

    static unsigned int seed = 0;
    std::normal_distribution<float> normalDistribution { static_cast<float>(Mean),
                                                         static_cast<float>(Sd) };
    std::mt19937 engine { ++seed };

    std::generate(newNoiseValueSpan.begin(), newNoiseValueSpan.end(),
                  [&]() { return normalDistribution(engine); });

    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);
    float* radiodensities = radioDensityArray->WritePointer(0, numberOfPoints);

    vtkFloatArray* gaussianNoiseArray = GetDeepCopiedArtifactArray(input, output, SubType::GAUSSIAN);
    float* gaussianNoiseValues = gaussianNoiseArray->WritePointer(0, numberOfPoints);

    auto addNoiseValues = [noiseValues = gaussianNoiseValues,
                           newNoiseValues = newNoiseValues,
                           radiodensities = radiodensities] (vtkIdType pointId, vtkIdType endPointId) {
        vtkIdType const startPointId = pointId;

        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += newNoiseValues[pointId];

        pointId = startPointId;
        for (; pointId < endPointId; pointId++)
            noiseValues[pointId] += newNoiseValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addNoiseValues);
}
