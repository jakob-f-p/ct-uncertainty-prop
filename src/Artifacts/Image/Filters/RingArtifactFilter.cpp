#include "RingArtifactFilter.h"
#include "../../../Utils/ImageDataUtils.h"
#include "../../../Modeling/CtDataSource.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>

vtkStandardNewMacro(RingArtifactFilter)

void RingArtifactFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Inner Radius" << InnerRadius << ")\n";
    os << indent << "Ring Width" << RingWidth << ")\n";

    os << indent << "Radiodensity Factor" << RadiodensityFactor << ")\n";
    os << indent << std::format("Dark Intensity Value: ({}, {}, {})",
                                Center[0], Center[1], Center[2]) << ")\n";
}

auto RingArtifactFilter::RequestInformation(vtkInformation* request,
                                            vtkInformationVector** inputVector,
                                            vtkInformationVector *outputVector) -> int {
    ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);

    AddArrayInformationToPointDataVector(SubType::RING, outputVector);

    return 1;
}

void RingArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
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

    vtkSMPTools::For(0, numberOfPoints, Algorithm { this, output, radiodensities, newArtifactValues });

    vtkFloatArray* ringArtifactArray = GetDeepCopiedArtifactArray(input, output, SubType::RING);
    float* ringArtifactValues = ringArtifactArray->WritePointer(0, numberOfPoints);

    auto addNoiseValues = [ringArtifactValues, newArtifactValues, radiodensities] (vtkIdType pointId, vtkIdType endPointId) {
        vtkIdType const startPointId = pointId;

        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += newArtifactValues[pointId];

        pointId = startPointId;
        for (; pointId < endPointId; pointId++)
            ringArtifactValues[pointId] += newArtifactValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addNoiseValues);
}

RingArtifactFilter::Algorithm::Algorithm(RingArtifactFilter* self,
                                         vtkImageData* volumeData,
                                         float const* radiodensities,
                                         float* artifactValues) :
        Self(self),
        VolumeData(volumeData),
        Spacing([this] {
            std::array<double, 3> spacing {};
            std::copy(VolumeData->GetSpacing(), std::next(VolumeData->GetSpacing(), 3), spacing.begin());
            return spacing;
        }()),
        UpdateDims([this] {
            std::array<int, 3> updateDims {};
            std::copy(VolumeData->GetDimensions(), std::next(VolumeData->GetDimensions(), 3), updateDims.begin());
            return updateDims;
        }()),
        StartPoint([this] {
            DoublePoint startPoint;
            VolumeData->GetPoint(0, startPoint.data());
            return startPoint;
        }()),
        Radiodensities(radiodensities),
        ArtifactValues(artifactValues),
        InnerRadius(Self->GetInnerRadius()),
        RingWidth(Self->GetRingWidth()),
        OuterRadius(InnerRadius + RingWidth),
        RadiodensityChangeFactor(Self->GetRadiodensityFactor() - 1.0F),
        Center(Self->GetCenterPoint()) {}

void RingArtifactFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    if (OuterRadius == 0.0 || RadiodensityChangeFactor == 0.0)
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

                bool const isRing = xyDistance >= InnerRadius && xyDistance <= OuterRadius;
                ArtifactValues[pointId] = isRing
                        ? Radiodensities[pointId] * RadiodensityChangeFactor
                        : 0.0;

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
