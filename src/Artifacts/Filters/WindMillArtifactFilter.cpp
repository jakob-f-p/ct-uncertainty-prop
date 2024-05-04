#include "WindMillArtifactFilter.h"
#include "../../ImageDataUtils.h"
#include "../../Modeling/CtDataSource.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <numbers>

vtkStandardNewMacro(WindMillArtifactFilter)

void WindMillArtifactFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Bright Angular Width" << BrightAngularWidth << ")\n";
    os << indent << "Dark Angular Width" << DarkAngularWidth << ")\n";

    os << indent << "Bright Intensity Value" << DarkIntensityValue << ")\n";
    os << indent << "Dark Intensity Value" << DarkIntensityValue << ")\n";
}

auto WindMillArtifactFilter::RequestInformation(vtkInformation* request,
                                                vtkInformationVector** inputVector,
                                                vtkInformationVector *outputVector) -> int {
    ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);

    AddArrayInformationToPointDataVector(SubType::WIND_MILL, outputVector);

    return 1;
}

void WindMillArtifactFilter::ExecuteDataWithImageInformation(vtkImageData* input,
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

    vtkFloatArray* windMillArtifactArray = GetDeepCopiedArtifactArray(input, output, SubType::WIND_MILL);
    float* windMillArtifactValues = windMillArtifactArray->WritePointer(0, numberOfPoints);

    auto addArtifactValues = [windMillArtifactValues, newArtifactValues, radiodensities] (vtkIdType pointId, vtkIdType endPointId) {
        vtkIdType const startPointId = pointId;

        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += newArtifactValues[pointId];

        pointId = startPointId;
        for (; pointId < endPointId; pointId++)
            windMillArtifactValues[pointId] += newArtifactValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addArtifactValues);
}

WindMillArtifactFilter::Algorithm::Algorithm(WindMillArtifactFilter* self, vtkImageData* volumeData, float* artifactValues) :
        Self(self),
        VolumeData(volumeData),
        Spacing(),
        UpdateDims(),
        ArtifactValues(artifactValues),
        BrightAngularWidth(vtkMath::RadiansFromDegrees(Self->GetBrightAngularWidth())),
        DarkAngularWidth(vtkMath::RadiansFromDegrees(Self->GetDarkAngularWidth())),
        CombinedAngularWidth(BrightAngularWidth + DarkAngularWidth),
        BrightDarkThreshold(CombinedAngularWidth == 0.0 ? 0.0 : BrightAngularWidth / CombinedAngularWidth),
        BrightIntensityValue(Self->GetBrightIntensityValue()),
        DarkIntensityValue(Self->GetDarkIntensityValue()),
        Center(Self->GetCenterPoint()) {
    std::copy(volumeData->GetSpacing(), std::next(volumeData->GetSpacing(), 3), Spacing.begin());
    std::copy(volumeData->GetDimensions(), std::next(volumeData->GetDimensions(), 3), UpdateDims.begin());
}

void WindMillArtifactFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    if (CombinedAngularWidth == 0.0)
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
                float const xFromCenter = static_cast<float>(point[0] - Center[0]);
                float const yFromCenter = static_cast<float>(point[1] - Center[1]);

                float const angleFromCenter = std::atan2(yFromCenter, xFromCenter) + static_cast<float>(std::numbers::pi);
                float const numberOfCombinedAngularWidths = angleFromCenter / CombinedAngularWidth;

                float integralPart;
                float const fractionalPart = std::modf(numberOfCombinedAngularWidths, &integralPart);

                bool const isDark = fractionalPart >= BrightDarkThreshold;
                ArtifactValues[pointId] = isDark ? DarkIntensityValue : BrightIntensityValue;

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
