#pragma once

#include "../Artifacts/Pipeline.h"
#include "PipelineParameterSpace.h"

#include <string>
#include <vector>


class PipelineGroup {
public:
    explicit PipelineGroup(Pipeline const& basePipeline);

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetBasePipeline() const noexcept -> Pipeline const&;

    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     ParameterSpanVariant&& parameterSpan) -> ParameterSpanVariant&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        ParameterSpanVariant const& parameterSpan) -> void;

private:
    std::string Name;
    Pipeline const& BasePipeline;
    std::vector<Pipeline> Pipelines;
    PipelineParameterSpace ParameterSpace;
};
