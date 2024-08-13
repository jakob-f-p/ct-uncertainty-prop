#include "CuppingArtifactFilter.h"
#include "../../../Utils/ImageDataUtils.h"
#include "../../../Modeling/CtDataSource.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <numbers>

vtkStandardNewMacro(CuppingArtifactFilter)

void CuppingArtifactFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Minimum Radiodensity Factor" << MinRadiodensityFactor << ")\n";
}

auto CuppingArtifactFilter::RequestInformation(vtkInformation* request,
                                               vtkInformationVector** inputVector,
                                               vtkInformationVector *outputVector) -> int {
    ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);

    AddArrayInformationToPointDataVector(SubType::CUPPING, outputVector);

    return 1;
}

void CuppingArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
                                                            vtkImageData* output,
                                                            vtkInformation* outInfo) {
    vtkIdType const numberOfPoints = output->GetNumberOfPoints();

    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);
    float* radiodensities = radioDensityArray->WritePointer(0, numberOfPoints);

    vtkNew<vtkFloatArray> newArtifactValueArray;
    newArtifactValueArray->SetNumberOfComponents(1);
    newArtifactValueArray->SetNumberOfTuples(numberOfPoints);
    newArtifactValueArray->FillValue(0.0F);
    float* newArtifactValues = newArtifactValueArray->WritePointer(0, numberOfPoints);

    vtkSMPTools::For(0, numberOfPoints, Algorithm { this, output, radiodensities, newArtifactValues });

    vtkFloatArray* cuppingArtifactArray = GetDeepCopiedArtifactArray(input, output, SubType::WIND_MILL);
    float* cuppingArtifactValues = cuppingArtifactArray->WritePointer(0, numberOfPoints);

    auto addArtifactValues = [cuppingArtifactValues, newArtifactValues, radiodensities] (vtkIdType pointId,
                                                                                         vtkIdType endPointId) {
        vtkIdType const startPointId = pointId;

        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += newArtifactValues[pointId];

        pointId = startPointId;
        for (; pointId < endPointId; pointId++)
            cuppingArtifactValues[pointId] += newArtifactValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addArtifactValues);
}

CuppingArtifactFilter::Algorithm::Algorithm(CuppingArtifactFilter* self,
                                            vtkImageData* volumeData,
                                            float const* radiodensities,
                                            float* artifactValues) :
        Self(self),
        VolumeData(volumeData),
        Spacing([this]() {
            std::array<double, 3> spacing {};
            std::copy(VolumeData->GetSpacing(), std::next(VolumeData->GetSpacing(), 3), spacing.begin());
            return spacing;
        }()),
        UpdateDims([this]() {
            std::array<int, 3> updateDims {};
            std::copy(VolumeData->GetDimensions(), std::next(VolumeData->GetDimensions(), 3), updateDims.begin());
            return updateDims;
        }()),
        StartPoint([this]() {
            DoublePoint startPoint;
            VolumeData->GetPoint(0, startPoint.data());
            return startPoint;
        }()),
        Radiodensities(radiodensities),
        ArtifactValues(artifactValues),
        MinRadiodensityFactor(Self->GetMinRadiodensityFactor()),
        RadiodensityFactorRange(1.0F - MinRadiodensityFactor),
        Center(Self->GetCenterPoint()),
        xyMaxDistance([&]() {
            std::array<double, 6> bounds {};
            VolumeData->GetBounds(bounds.data());
            double const xMaxDistance = std::max(std::abs(bounds[1] - Center[0]),
                                                 std::abs(Center[0] - bounds[0]));
            double const yMaxDistance = std::max(std::abs(bounds[3] - Center[1]),
                                                 std::abs(Center[1] - bounds[2]));
            return static_cast<float>(std::sqrt(xMaxDistance * xMaxDistance + yMaxDistance * yMaxDistance));
        }()) {}

void CuppingArtifactFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    if (MinRadiodensityFactor == 1.0F)
        return;

    DoublePoint startPoint;
    VolumeData->GetPoint(pointId, startPoint.data());

    std::array<int, 3> const startPointCoordinates = PointIdToDimensionCoordinates(pointId, UpdateDims);
    std::array<int, 3> const endPointCoordinates = PointIdToDimensionCoordinates(endPointId, UpdateDims);
    auto [ x1, y1, z1 ] = startPointCoordinates;
    auto lastValidPointCoordinates = GetDecrementedCoordinates(endPointCoordinates, UpdateDims);
    auto [ x2, y2, z2 ] = lastValidPointCoordinates;

    DoublePoint point = StartPoint;
    point[0] += x1 * Spacing[0];
    point[1] += y1 * Spacing[1];
    point[2] += z1 * Spacing[2];

    for (vtkIdType z = z1; z <= z2; z++) {
        vtkIdType y = 0;
        if (z == z1)
            y = y1;

        vtkIdType yEnd = UpdateDims[1] - 1;
        if (z == z2)
            yEnd = y2;

        for (; y <= yEnd; y++) {
            vtkIdType x = 0;
            if (z == z1 && y == y1)
                x = x1;

            vtkIdType xEnd = UpdateDims[0] - 1;
            if (z == z2 && y == y2)
                xEnd = x2;

            for (; x <= xEnd; x++) {
                auto const xDistance = static_cast<float>(point[0] - Center[0]);
                auto const yDistance = static_cast<float>(point[1] - Center[1]);
                float const xyDistance = std::sqrt(xDistance * xDistance + yDistance * yDistance);

                float const relativeDistance = xyDistance / xyMaxDistance;
                float const factor = relativeDistance * RadiodensityFactorRange + MinRadiodensityFactor;

                ArtifactValues[pointId] = Radiodensities[pointId] * (factor - 1.0F);

                pointId++;

                point[0] += Spacing[0];
            }

            point[0] = StartPoint[0];
            point[1] += Spacing[1];
        }

        point[0] = StartPoint[0];
        point[1] = StartPoint[1];
        point[2] += Spacing[2];
    }
}
