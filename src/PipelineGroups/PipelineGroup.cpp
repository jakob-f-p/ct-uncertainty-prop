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

auto PipelineGroup::GetMTime() const noexcept -> vtkMTimeType {
    return ParameterSpace->GetMTime();
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

auto PipelineGroup::GetImageData() -> TimeStampedData<std::vector<PipelineImageData*>> {
    if (!Batch)
        throw std::runtime_error("Cannot get image data. Batch has not been created");

    return Batch->GetImageData();
}

auto PipelineGroup::GetFeatureData() const -> TimeStampedDataRef<FeatureData> {
    if (!Batch)
        throw std::runtime_error("Cannot get feature data. Batch has not been created");

    return Batch->GetFeatureData();
}

auto PipelineGroup::GetPcaData() const -> TimeStampedDataRef<SampleCoordinateData> {
    if (!Batch)
        throw std::runtime_error("Cannot get PCA data. Batch has not been created");

    return Batch->GetPcaData();
}

auto PipelineGroup::GetTsneData() const -> TimeStampedDataRef<SampleCoordinateData> {
    if (!Batch)
        throw std::runtime_error("Cannot get t-SNE data. Batch has not been created");

    return Batch->GetTsneData();
}

auto PipelineGroup::SetTsneData(SampleCoordinateData&& tsneData) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot set t-SNE data. Batch has not been created");

    Batch->SetTsneData(std::move(tsneData));
}

auto PipelineGroup::GetDataStatus() const noexcept -> DataStatus {
    return Batch
            ? Batch->GetDataStatus()
            : DataStatus {};
}

auto PipelineGroup::ExportGeneratedImages(std::filesystem::path const& exportDir,
                                          PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot export image data. Batch has not been created");

    Batch->ExportGeneratedImages(exportDir, callback);
}

auto PipelineGroup::ImportImages(std::vector<std::filesystem::path> const& importFilePaths,
                                 PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot import image data. Batch has not been created");

    Batch->ImportImages(importFilePaths, callback);
}

auto PipelineGroup::ImportFeatures(std::filesystem::path const& importFilePath,
                                   PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot import feature data. Batch has not been created");

    Batch->ImportFeatures(importFilePath, callback);
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
