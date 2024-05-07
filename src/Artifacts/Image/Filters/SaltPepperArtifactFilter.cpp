#include "SaltPepperArtifactFilter.h"

#include <vtkMinimalStandardRandomSequence.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(SaltPepperArtifactFilter)

void SaltPepperArtifactFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Amount of Salt: " << SaltAmount << ")\n";
    os << indent << "Amount of Pepper: " << PepperAmount << ")\n";

    os << indent << "Salt Intensity Value: " << SaltIntensityValue << ")\n";
    os << indent << "Pepper Intensity Value: " << PepperIntensityValue << ")\n";
}

auto SaltPepperArtifactFilter::RequestInformation(vtkInformation* request,
                                                 vtkInformationVector** inputVector,
                                                 vtkInformationVector *outputVector) -> int {
    ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);

    AddArrayInformationToPointDataVector(SubType::SALT_PEPPER, outputVector);

    return 1;
}

struct FillWithRandomIndices {
    std::vector<vtkIdType>& Indices;
    int NumberOfIndices;
    vtkIdType NumberOfPoints;

    auto operator() () noexcept -> void {
        vtkNew<vtkMinimalStandardRandomSequence> random;

        Indices.reserve(NumberOfIndices);

        while (Indices.size() != NumberOfIndices) {
            vtkIdType const idx = vtkMath::Floor(random->GetNextRangeValue(0.0, NumberOfPoints));

            if (std::find(Indices.begin(), Indices.end(), idx) == Indices.end())
                Indices.push_back(idx);
        }

        std::sort(Indices.begin(), Indices.end());
    }
};

struct AddNoise {
    std::vector<vtkIdType> const& Indices;
    float* SaltPepperNoiseValues;
    float NoiseValue;

    void operator()(vtkIdType indicesIdx, vtkIdType endIndicesIdx) const {

        for (; indicesIdx < endIndicesIdx; indicesIdx++)
            SaltPepperNoiseValues[Indices[indicesIdx]] = NoiseValue;
    }
};

void SaltPepperArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
                                                               vtkImageData* output,
                                                               vtkInformation* outInfo) {
    vtkIdType const numberOfPoints = output->GetNumberOfPoints();

    int const numberOfSaltIndices = vtkMath::Ceil(numberOfPoints * SaltAmount);
    std::vector<vtkIdType> saltIndices;

    int const numberOfPepperIndices = vtkMath::Ceil(numberOfPoints * PepperAmount);
    std::vector<vtkIdType> pepperIndices;

    std::thread saltIndicesThread   (FillWithRandomIndices{ saltIndices,   numberOfSaltIndices,   numberOfPoints });
    std::thread pepperIndicesThread (FillWithRandomIndices{ pepperIndices, numberOfPepperIndices, numberOfPoints });
    saltIndicesThread.join();
    pepperIndicesThread.join();

    vtkNew<vtkFloatArray> newNoiseValueArray;
    newNoiseValueArray->SetNumberOfComponents(1);
    newNoiseValueArray->SetNumberOfTuples(numberOfPoints);
    newNoiseValueArray->FillValue(0.0F);
    float* newNoiseValues = newNoiseValueArray->WritePointer(0, numberOfPoints);

    vtkSMPTools::For(0, numberOfSaltIndices,   AddNoise { saltIndices,   newNoiseValues, SaltIntensityValue });
    vtkSMPTools::For(0, numberOfPepperIndices, AddNoise { pepperIndices, newNoiseValues, PepperIntensityValue });


    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);
    float* radiodensities = radioDensityArray->WritePointer(0, numberOfPoints);

    vtkFloatArray* saltPepperNoiseArray = GetDeepCopiedArtifactArray(input, output, SubType::SALT_PEPPER);
    float* saltPepperNoiseValues = saltPepperNoiseArray->WritePointer(0, numberOfPoints);

    auto addNoiseValues = [noiseValues = saltPepperNoiseValues,
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
