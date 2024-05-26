#pragma once

#include "../Artifacts/Pipeline.h"
#include "PipelineParameterSpace.h"

#include <string>
#include <vector>


class PipelineGroup {
public:
    explicit PipelineGroup(Pipeline const& basePipeline, std::string name = "");

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetBasePipeline() const noexcept -> Pipeline const&;

    [[nodiscard]] auto
    GetParameterSpace() noexcept -> PipelineParameterSpace&;

    [[nodiscard]] auto
    GetParameterSpace() const noexcept -> PipelineParameterSpace const&;

    [[nodiscard]] auto
    CreatePipelines() const noexcept -> std::vector<Pipeline>;

    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

private:
    std::string Name;
    Pipeline const& BasePipeline;
    std::vector<Pipeline> Pipelines;
    PipelineParameterSpace ParameterSpace;

    static uint16_t PipelineGroupId;
};


class PipelineBatch {
public:
    explicit PipelineBatch(PipelineGroup const& pipelineGroup);

private:
    PipelineGroup const& Group;
    std::vector<Pipeline> Pipelines;
};
