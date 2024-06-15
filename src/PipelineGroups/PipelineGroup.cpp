#include "PipelineGroup.h"

#include "PipelineBatch.h"
#include "PipelineParameterSpace.h"
#include "PipelineParameterSpaceState.h"

#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"

#include <memory>


PipelineGroup::PipelineGroup(Pipeline const& basePipeline, std::string name) :
        Name(name.empty()
             ? "Pipeline Group " + std::to_string(PipelineGroupId++)
             : std::move(name)),
        GroupId(PipelineGroupId++),
        BasePipeline(basePipeline),
        ParameterSpace(new PipelineParameterSpace()),
        Batch(new PipelineBatch(*this)) {};

PipelineGroup::~PipelineGroup() = default;

auto PipelineGroup::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroup::GetBasePipeline() const noexcept -> Pipeline const& {
    return BasePipeline;
}

auto PipelineGroup::GetParameterSpace() noexcept -> PipelineParameterSpace& {
    return *ParameterSpace;
}

auto PipelineGroup::GetParameterSpace() const noexcept -> PipelineParameterSpace const& {
    return *ParameterSpace;
}

auto PipelineGroup::GenerateImages(ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot generate images. Batch has not been created");

    Batch->UpdateParameterSpaceStates();

    Batch->GenerateImages(callback);
}

auto PipelineGroup::ExportImages(PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot export. Batch has not been created");

    Batch->ExportImages(callback);
}

auto PipelineGroup::ExtractFeatures(PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot extract features. Batch has not been created");

    Batch->ExtractFeatures(callback);
}

auto PipelineGroup::DoPCA(uint8_t numberOfDimensions) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot do PCA. Batch has not been created");

    Batch->DoPCA(numberOfDimensions);
}

auto PipelineGroup::GetImageData() -> std::vector<PipelineImageData*> {
    if (!Batch)
        throw std::runtime_error("Cannot get image data. Batch has not been created");

    return Batch->GetImageData();
}

auto PipelineGroup::GetFeatureData() const -> FeatureData const& {
    if (!Batch)
        throw std::runtime_error("Cannot get feature data. Batch has not been created");

    return Batch->GetFeatureData();
}

auto PipelineGroup::GetPcaData() const -> SampleCoordinateData const& {
    if (!Batch)
        throw std::runtime_error("Cannot get PCA data. Batch has not been created");

    return Batch->GetPcaData();
}

auto PipelineGroup::GetTsneData() const -> SampleCoordinateData const& {
    if (!Batch)
        throw std::runtime_error("Cannot get t-SNE data. Batch has not been created");

    return Batch->GetTsneData();
}

auto PipelineGroup::SetTsneData(SampleCoordinateData&& tsneData) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot set t-SNE data. Batch has not been created");

    Batch->SetTsneData(std::move(tsneData));
}

auto PipelineGroup::DataHasBeenGenerated() const noexcept -> bool {
    return Batch && Batch->DataHasBeenGenerated();
}

auto PipelineGroup::GetDataMTime() const noexcept -> vtkMTimeType {
    return Batch ? Batch->GetDataMTime() : 0;
}

auto PipelineGroup::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan& {
    return ParameterSpace->AddParameterSpan(artifactVariantPointer, std::move(parameterSpan));
}

auto PipelineGroup::RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                        PipelineParameterSpan const& parameterSpan) -> void {
    ParameterSpace->RemoveParameterSpan(artifactVariantPointer, parameterSpan);
}

uint16_t PipelineGroup::PipelineGroupId = 0;
