#include "StructureArtifactsFilter.h"

#include "StructureArtifactListCollection.h"
#include "../../Modeling/CtStructureTree.h"
#include "../../Utils/ImageDataUtils.h"
#include "../../Utils/Overload.h"

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

auto StructureArtifactsFilter::SetCtStructureTree(CtStructureTree const& ctStructureTree) -> void {
    if (&ctStructureTree == StructureTree)
        return;

    StructureTree = &ctStructureTree;
    Modified();
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


template<std::size_t N>
struct num { static const constexpr auto value = N; };

template<typename F, std::size_t... Is>
void for_(F func, std::index_sequence<Is...>) { (func(num<Is>{}), ...); }

template <std::size_t N, typename F>
void for_(F func) { for_(func, std::make_index_sequence<N>()); }

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


    auto* structureIdArray = vtkTypeUInt16Array::SafeDownCast(
            output->GetPointData()->GetAbstractArray("BasicStructureIds"));

    vtkNew<vtkFloatArray> radiodensityArray;
    auto* inputRadiodensityArray = vtkFloatArray::SafeDownCast(
            input->GetPointData()->GetAbstractArray("Radiodensities"));
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
    metallicArtifactArray->SetName(GetArrayName(SubType::METAL).data());
    metallicArtifactArray->SetNumberOfTuples(numberOfPoints);
    metallicArtifactArray->FillValue(0.0F);
    output->GetPointData()->AddArray(metallicArtifactArray);

    vtkNew<vtkFloatArray> windmillArtifactArray;
    windmillArtifactArray->SetNumberOfComponents(1);
    windmillArtifactArray->SetName(GetArrayName(SubType::WINDMILL).data());
    windmillArtifactArray->SetNumberOfTuples(numberOfPoints);
    windmillArtifactArray->FillValue(0.0F);
    output->GetPointData()->AddArray(windmillArtifactArray);

    StructureId const* const structureIds = structureIdArray->WritePointer(0, numberOfPoints);
    float* const radiodensities = radiodensityArray->WritePointer(0, numberOfPoints);
    float* const motionValues = motionArtifactArray->WritePointer(0, numberOfPoints);
    float* const metallicValues = metallicArtifactArray->WritePointer(0, numberOfPoints);
    float* const windmillValues = windmillArtifactArray->WritePointer(0, numberOfPoints);
    std::array<float* const,
               StructureArtifactDetails::GetNumberOfSubTypeValues()> const artifactArrays { motionValues,
                                                                                            metallicValues,
                                                                                            windmillValues };
    for_<StructureArtifactDetails::GetNumberOfSubTypeValues()>([this, output, radiodensities,
                                                                structureIds, artifactArrays,
                                                                numberOfPoints](auto iCounter) {

        auto const i = iCounter.value;
        auto const ArtifactType = static_cast<SubType>(i);

        TreeStructureArtifactListCollection::StructureArtifactsMap const artifactsMap
                = StructureArtifactCollection->GetStructureArtifacts<ArtifactType>();

        for (auto const& [ structure, artifacts ] : artifactsMap) {
            for (auto const& artifact : artifacts) {
                Algorithm algorithm { this, StructureTree, output, &structure.get(), &artifact.get(),
                                      radiodensities, structureIds, artifactArrays[i] };

                vtkSMPTools::For(0, numberOfPoints, algorithm);
            }
        }
    });

    auto addArtifactValues = [=](vtkIdType pointId, vtkIdType endPointId) {
        for (; pointId < endPointId; pointId++) {
            float const delta = motionValues[pointId] + metallicValues[pointId] + windmillValues[pointId];
            radiodensities[pointId] += delta;

            if (motionValues[pointId] > 0.0F)
                radiodensities[pointId] += 1000.0F;
        }
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
                                               CtStructureTree const* structureTree,
                                               vtkImageData* volumeData,
                                               CtStructureVariant const* structureVariant,
                                               StructureArtifact const* structureArtifact,
                                               float* radiodensities,
                                               StructureId const* basicStructureIds,
                                               float* const artifactValues) :
        Self(self),
        StructureTree(structureTree),
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
        StructureVariant(structureVariant),
        MaxStructureRadiodensity(StructureTree->GetMaxTissueValue(*structureVariant)),
        StructureArtifact_(structureArtifact),
        Radiodensities(radiodensities),
        BasicStructureIds(basicStructureIds),
        StructureArtifactIds(StructureTree->GetBasicStructureIds(*StructureVariant)),
        ArtifactValues(artifactValues) {
    std::copy(volumeData->GetSpacing(), std::next(volumeData->GetSpacing(), 3), Spacing.begin());
    std::copy(volumeData->GetDimensions(), std::next(volumeData->GetDimensions(), 3), UpdateDims.begin());
}

//struct Calculate {
//    DoublePoint const StartPoint;
//    std::array<double, 3> const Spacing;
//    std::array<int, 3> const UpdateDims;
//    int const X1, Y1, Z1;
//    int const X2, Y2, Z2;
//    vtkIdType const PointId;
//    StructureId const* BasicStructureIds;
//    float* const StructureArtifactValues;
//
//    template<typename EvaluationFunction>
//    auto
//    operator()(StructureArtifact::SubType subType,
//               std::vector<StructureId> const& artifactStructureIds,
//               EvaluationFunction evaluation) const noexcept -> void {
//
//        vtkIdType pointId = PointId;
//
//        DoublePoint point = StartPoint;
//        point[0] += X1 * Spacing[0];
//        point[1] += Y1 * Spacing[1];
//        point[2] += Z1 * Spacing[2];
//
//        for (vtkIdType z = Z1; z <= Z2; z++) {
//            vtkIdType y = 0;
//            if (z == Z1)
//                y = Y1;
//
//            vtkIdType yEnd = UpdateDims[1] - 1;
//            if (z == Z2)
//                yEnd = Y2;
//
//            for (; y <= yEnd; y++) {
//                vtkIdType x = 0;
//                if (z == Z1 && y == Y1)
//                    x = X1;
//
//                vtkIdType xEnd = UpdateDims[0] - 1;
//                if (z == Z2 && y == Y2)
//                    xEnd = X2;
//
//                for (; x <= xEnd; x++) {
//                    bool const pointOccupiedByStructure
//                            = std::any_of(artifactStructureIds.cbegin(), artifactStructureIds.cend(),
//                                          [&](StructureId id) { return id == BasicStructureIds[pointId]; });
//
//                    StructureArtifactValues[pointId] += evaluation(point, pointOccupiedByStructure);
//
//                    pointId++;
//
//                    point[0] += Spacing[0];
//                }
//
//                point[0] = StartPoint[0];
//                point[1] += Spacing[1];
//            }
//
//            point[0] = StartPoint[0];
//            point[1] = StartPoint[1];
//            point[2] += Spacing[2];
//        }
//    }
//};

void StructureArtifactsFilter::Algorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
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
                bool const pointOccupiedByStructure
                        = std::any_of(StructureArtifactIds.cbegin(), StructureArtifactIds.cend(),
                                      [&](StructureId id) { return id == BasicStructureIds[pointId]; });

                ArtifactValues[pointId] += StructureArtifact_->EvaluateAtPosition(point,
                                                                                  MaxStructureRadiodensity,
                                                                                  pointOccupiedByStructure,
                                                                                  *StructureTree,
                                                                                  *StructureVariant,
                                                                                  Spacing);

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
