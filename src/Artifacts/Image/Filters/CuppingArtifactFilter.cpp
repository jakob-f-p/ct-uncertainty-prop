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

    os << indent << "Dark Intensity Value" << DarkIntensityValue << ")\n";
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

    vtkNew<vtkFloatArray> newArtifactValueArray;
    newArtifactValueArray->SetNumberOfComponents(1);
    newArtifactValueArray->SetNumberOfTuples(numberOfPoints);
    newArtifactValueArray->FillValue(0.0F);
    float* newArtifactValues = newArtifactValueArray->WritePointer(0, numberOfPoints);

    vtkSMPTools::For(0, numberOfPoints, Algorithm { this, output, newArtifactValues });

    vtkFloatArray* radioDensityArray = GetRadiodensitiesArray(output);
    float* radiodensities = radioDensityArray->WritePointer(0, numberOfPoints);

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
                                            float* artifactValues) :
        Self(self),
        VolumeData(volumeData),
        Spacing(),
        UpdateDims(),
        ArtifactValues(artifactValues),
        DarkIntensityValue(Self->GetDarkIntensityValue()),
        xyMaxDistance([&]() {
            std::array<double, 6> bounds {};
            VolumeData->GetBounds(bounds.data());
            double const xMaxDistance = bounds[1] - bounds[0];
            double const yMaxDistance = bounds[3] - bounds[2];
            return static_cast<float>(std::sqrt(xMaxDistance * xMaxDistance + yMaxDistance * yMaxDistance));
        }()),
        Center(Self->GetCenterPoint()) {
    std::copy(volumeData->GetSpacing(), std::next(volumeData->GetSpacing(), 3), Spacing.begin());
    std::copy(volumeData->GetDimensions(), std::next(volumeData->GetDimensions(), 3), UpdateDims.begin());
}

void CuppingArtifactFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    DoublePoint startPoint;
    VolumeData->GetPoint(pointId, startPoint.data());

    std::array<int, 3> const startPointCoordinates = PointIdToDimensionCoordinates(pointId, UpdateDims);
    std::array<int, 3> const endPointCoordinates = PointIdToDimensionCoordinates(endPointId, UpdateDims);
    auto [ x1, y1, z1 ] = startPointCoordinates;
    auto lastValidPointCoordinates = GetDecrementedCoordinates(endPointCoordinates, UpdateDims);
    auto [ x2, y2, z2 ] = lastValidPointCoordinates;

    DoublePoint point = startPoint;
    for (vtkIdType z = z1; z <= z2; z++) {
        vtkIdType y = 0;
        if (z == z1)
            y = y1;

        vtkIdType yEnd = UpdateDims[1] - 1;
        if (z == z2 - 1)
            yEnd = y2;

        for (; y <= yEnd; y++) {
            vtkIdType x = 0;
            if (y == y1)
                x = x1;

            vtkIdType xEnd = UpdateDims[0] - 1;
            if (y == y2 - 1)
                xEnd = x2;

            for (; x <= xEnd; x++) {
                float const xDistance = static_cast<float>(point[0] - Center[0]);
                float const yDistance = static_cast<float>(point[1] - Center[1]);

                float const xyDistance = std::sqrt(xDistance * xDistance + yDistance * yDistance);
                float const relativeDistance = xyDistance / xyMaxDistance;

                ArtifactValues[pointId] = DarkIntensityValue * (1.0F - relativeDistance);

                pointId++;

                point[0] += Spacing[0];
            }
            point[0] = startPoint[0];
            point[1] += Spacing[1];
        }
        point[0] = startPoint[0];
        point[1] = startPoint[1];
        point[2] += Spacing[2];
    }
}
