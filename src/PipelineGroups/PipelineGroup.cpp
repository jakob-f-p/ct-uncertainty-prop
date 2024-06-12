#include "PipelineGroup.h"

#include <memory>

#include "PipelineBatch.h"
#include "PipelineParameterSpace.h"
#include "PipelineParameterSpaceState.h"

#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"


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
    Batch = std::make_unique<PipelineBatch>(*this);

    Batch->GenerateImages(callback);
}

auto PipelineGroup::ExportImages(PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot export. Batch has not been generated");

    Batch->ExportImages(callback);
}

auto PipelineGroup::ExtractFeatures(PipelineGroup::ProgressEventCallback const& callback) -> void {
    if (!Batch)
        throw std::runtime_error("Cannot extract features. Batch has not been generated");

    Batch->ExtractFeatures(callback);
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
