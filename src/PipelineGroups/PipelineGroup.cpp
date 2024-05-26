#include "PipelineGroup.h"

#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"

PipelineGroup::PipelineGroup(Pipeline const& basePipeline, std::string name) :
        Name(name.empty()
             ? "Pipeline Group " + std::to_string(PipelineGroupId++)
             : std::move(name)),
        BasePipeline(basePipeline) {};

auto PipelineGroup::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroup::GetBasePipeline() const noexcept -> Pipeline const& {
    return BasePipeline;
}

auto PipelineGroup::GetParameterSpace() noexcept -> PipelineParameterSpace& {
    return ParameterSpace;
}

auto PipelineGroup::GetParameterSpace() const noexcept -> PipelineParameterSpace const& {
    return ParameterSpace;
}

auto PipelineGroup::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan& {
    return ParameterSpace.AddParameterSpan(artifactVariantPointer, std::move(parameterSpan));
}

auto PipelineGroup::RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                        PipelineParameterSpan const& parameterSpan) -> void {
    ParameterSpace.RemoveParameterSpan(artifactVariantPointer, parameterSpan);
}

uint16_t PipelineGroup::PipelineGroupId = 1;

auto PipelineGroup::CreatePipelines() const noexcept -> std::vector<Pipeline> {
    std::vector<Pipeline> pipelines;
    pipelines.reserve(ParameterSpace.GetNumberOfPipelines());



    return pipelines;
}


PipelineBatch::PipelineBatch(PipelineGroup const& pipelineGroup) :
        Group(pipelineGroup),
        Pipelines(pipelineGroup.CreatePipelines()) {}
