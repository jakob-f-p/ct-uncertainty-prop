#include "StructureArtifactsFilter.h"

#include "StructureArtifactListCollection.h"
#include "../../Utils/ImageDataUtils.h"

#include <vtkErrorCode.h>
#include <vtkDataObject.h>
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTypeUInt16Array.h>

vtkStandardNewMacro(StructureArtifactsFilter)

void StructureArtifactsFilter::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Structure Artifact Collection: [" << StructureArtifactCollection << ")\n";
}

auto StructureArtifactsFilter::GetMTime() -> vtkMTimeType {
    vtkMTimeType const mTime = Superclass::GetMTime();
    vtkMTimeType const treeMTime = StructureArtifactCollection ? StructureArtifactCollection->GetMTime() : 0;

    return std::max({ mTime, treeMTime });
}

auto StructureArtifactsFilter::CreateDefaultExecutive() -> vtkExecutive* {
    return vtkStreamingDemandDrivenPipeline::New();
}

auto StructureArtifactsFilter::GetStructureArtifactCollection() const -> TreeStructureArtifactListCollection* {
    return StructureArtifactCollection;
}

auto StructureArtifactsFilter::SetStructureArtifactCollection(
        TreeStructureArtifactListCollection* structureArtifactCollection) -> void {
    if (structureArtifactCollection == StructureArtifactCollection)
        return;

    StructureArtifactCollection = structureArtifactCollection;
    Modified();
}

auto StructureArtifactsFilter::RequestInformation(vtkInformation* request,
                                                  vtkInformationVector** inputVector,
                                                  vtkInformationVector *outputVector) -> int {

    CopyInputArrayAttributesToOutput(request, inputVector, outputVector);

    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    outInfo->CopyEntry(inInfo, vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
    outInfo->CopyEntry(inInfo, vtkDataObject::ORIGIN());
    outInfo->CopyEntry(inInfo, vtkDataObject::SPACING());
    outInfo->CopyEntry(inInfo, vtkDataObject::POINT_DATA_VECTOR());

    outInfo->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    return 1;
}

auto StructureArtifactsFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector,
                                           vtkInformationVector* outputVector) -> int {

    const int outputPort = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());

    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation* outInfo = outputVector->GetInformationObject(outputPort);
    vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    output->SetExtent(input->GetExtent());

    output->GetPointData()->PassData(input->GetPointData());

    SetErrorCode(vtkErrorCode::NoError);


    auto* structureIdArray = vtkTypeUInt16Array::SafeDownCast(output->GetPointData()->GetAbstractArray("BasicStructureIds"));

    vtkNew<vtkFloatArray> radiodensityArray;
    auto* inputRadiodensityArray = vtkFloatArray::SafeDownCast(input->GetPointData()->GetAbstractArray("Radiodensities"));
    radiodensityArray->DeepCopy(inputRadiodensityArray);
    output->GetPointData()->AddArray(radiodensityArray);

    vtkIdType const numberOfPoints = output->GetNumberOfPoints();
    vtkNew<vtkFloatArray> motionArtifactArray;
    motionArtifactArray->SetNumberOfComponents(1);
    motionArtifactArray->SetName(GetArrayName(SubType::MOTION).data());
    motionArtifactArray->SetNumberOfTuples(numberOfPoints);
    motionArtifactArray->FillValue(0.0F);
    output->GetPointData()->AddArray(motionArtifactArray);

    vtkNew<vtkFloatArray> metallicArtifactArray;
    metallicArtifactArray->SetNumberOfComponents(1);
    metallicArtifactArray->SetName(GetArrayName(SubType::METALLIC).data());
    metallicArtifactArray->SetNumberOfTuples(numberOfPoints);
    metallicArtifactArray->FillValue(0.0F);
    output->GetPointData()->AddArray(metallicArtifactArray);

    vtkNew<vtkFloatArray> streakingArtifactArray;
    streakingArtifactArray->SetNumberOfComponents(1);
    streakingArtifactArray->SetName(GetArrayName(SubType::STREAKING).data());
    streakingArtifactArray->SetNumberOfTuples(numberOfPoints);
    streakingArtifactArray->FillValue(0.0F);
    output->GetPointData()->AddArray(streakingArtifactArray);

    StructureId const* const structureIds = structureIdArray->WritePointer(0, numberOfPoints);
    float* const radiodensities = radiodensityArray->WritePointer(0, numberOfPoints);
    float* const motionValues = motionArtifactArray->WritePointer(0, numberOfPoints);
    float* const metallicValues = metallicArtifactArray->WritePointer(0, numberOfPoints);
    float* const streakingValues = streakingArtifactArray->WritePointer(0, numberOfPoints);

    Algorithm algorithm(this, output, StructureArtifactCollection, radiodensities, structureIds,
                        { streakingValues, metallicValues, motionValues });

    vtkSMPTools::For(0, numberOfPoints, algorithm);

    auto addArtifactValues = [=](vtkIdType pointId, vtkIdType endPointId) {
        for (; pointId < endPointId; pointId++)
            radiodensities[pointId] += streakingValues[pointId] + metallicValues[pointId] + motionValues[pointId];
    };
    vtkSMPTools::For(0, numberOfPoints, addArtifactValues);

    if (GetErrorCode() != 0U)
        return 0;

    return 1;
}

auto StructureArtifactsFilter::GetArrayName(StructureArtifactsFilter::SubType subType) noexcept -> std::string {
    return StructureArtifactDetails::SubTypeToString(subType);
}

StructureArtifactsFilter::Algorithm::Algorithm(StructureArtifactsFilter* self,
                                               vtkImageData* volumeData,
                                               TreeStructureArtifactListCollection* treeArtifacts,
                                               float* radiodensities,
                                               StructureId const* basicStructureIds,
                                               std::array<float* const, 3> structureArtifactValues) :
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
        TreeArtifacts(treeArtifacts),
        Radiodensities(radiodensities),
        BasicStructureIds(basicStructureIds),
        StructureArtifactValues(structureArtifactValues) {
    std::copy(volumeData->GetSpacing(), std::next(volumeData->GetSpacing(), 3), Spacing.begin());
    std::copy(volumeData->GetDimensions(), std::next(volumeData->GetDimensions(), 3), UpdateDims.begin());
}

struct Calculate {
    DoublePoint const StartPoint;
    std::array<double, 3> const Spacing;
    std::array<int, 3> const UpdateDims;
    int const X1, Y1, Z1;
    int const X2, Y2, Z2;
    vtkIdType const PointId;
    StructureId const* BasicStructureIds;
    std::array<float* const, StructureArtifactDetails::GetNumberOfSubTypeValues()> StructureArtifactValues;

    template<typename EvaluationFunction>
    auto
    operator()(StructureArtifact::SubType subType,
               std::vector<StructureId> const& artifactStructureIds,
               EvaluationFunction evaluation) const noexcept -> void {

        vtkIdType pointId = PointId;

        float* const artifactValues = StructureArtifactValues[static_cast<uint8_t>(subType)];

        DoublePoint point = StartPoint;
        point[0] += X1 * Spacing[0];
        point[1] += Y1 * Spacing[1];
        point[2] += Z1 * Spacing[2];

        for (vtkIdType z = Z1; z <= Z2; z++) {
            vtkIdType y = 0;
            if (z == Z1)
                y = Y1;

            vtkIdType yEnd = UpdateDims[1] - 1;
            if (z == Z2)
                yEnd = Y2;

            for (; y <= yEnd; y++) {
                vtkIdType x = 0;
                if (z == Z1 && y == Y1)
                    x = X1;

                vtkIdType xEnd = UpdateDims[0] - 1;
                if (z == Z2 && y == Y2)
                    xEnd = X2;

                for (; x <= xEnd; x++) {
                    bool const pointOccupiedByStructure
                            = std::any_of(artifactStructureIds.cbegin(), artifactStructureIds.cend(),
                                          [&](StructureId id) { return id == BasicStructureIds[pointId]; });

                    artifactValues[pointId] += evaluation(point, pointOccupiedByStructure);

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
};

void StructureArtifactsFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    std::array<int, 3> const startPointCoordinates = PointIdToDimensionCoordinates(pointId, UpdateDims);
    std::array<int, 3> const endPointCoordinates = PointIdToDimensionCoordinates(endPointId, UpdateDims);
    auto lastValidPointCoordinates = GetDecrementedCoordinates(endPointCoordinates, UpdateDims);
    auto [ x1, y1, z1 ] = startPointCoordinates;
    auto [ x2, y2, z2 ] = lastValidPointCoordinates;

    TreeArtifacts->ArtifactValue( Calculate { StartPoint, Spacing, UpdateDims,
                                              x1, y1, z1, x2, y2, z2, pointId,
                                              BasicStructureIds, StructureArtifactValues });
}
