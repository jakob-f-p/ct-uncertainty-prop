#include "SaltPepperArtifactFilter.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>

#include <random>

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

    auto operator() () const noexcept -> void {
        static unsigned int seed = 0;

        std::uniform_int_distribution<vtkIdType> uniformDistribution { 0, NumberOfPoints - 1 };
        std::minstd_rand engine { ++seed };

        Indices.resize(NumberOfIndices, std::numeric_limits<vtkIdType>::max());

        for (auto IdxsEndIt = Indices.begin(); IdxsEndIt != Indices.end();) {
            std::generate(IdxsEndIt, Indices.end(),
                          [&] { return uniformDistribution(engine); });
            std::ranges::sort(Indices);
            IdxsEndIt = std::ranges::unique(Indices).begin();
        }
    }
};

struct AddNoise {
    std::vector<vtkIdType> const& Indices;
    float const* Radiodensities;
    float* SaltPepperNoiseValues;
    float NoiseValue;

    void operator()(vtkIdType indicesIdx, vtkIdType endIndicesIdx) const {

        for (; indicesIdx < endIndicesIdx; indicesIdx++)
            SaltPepperNoiseValues[Indices[indicesIdx]] = NoiseValue - Radiodensities[Indices[indicesIdx]];
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

    vtkNew<vtkFloatArray> const newNoiseValueArray;
    newNoiseValueArray->SetNumberOfComponents(1);
    newNoiseValueArray->SetNumberOfTuples(numberOfPoints);
    newNoiseValueArray->FillValue(0.0F);
    float* newNoiseValues = newNoiseValueArray->WritePointer(0, numberOfPoints);

    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);
    float* radiodensities = radioDensityArray->WritePointer(0, numberOfPoints);

    vtkSMPTools::For(0, numberOfSaltIndices,   AddNoise { saltIndices,   radiodensities,
                                                          newNoiseValues, SaltIntensityValue });
    vtkSMPTools::For(0, numberOfPepperIndices, AddNoise { pepperIndices, radiodensities,
                                                          newNoiseValues, PepperIntensityValue });


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
