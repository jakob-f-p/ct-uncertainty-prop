#include "PipelineGroup.h"

#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"

PipelineGroup::PipelineGroup(Pipeline const& basePipeline) :
        BasePipeline(basePipeline) {};

auto PipelineGroup::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroup::GetBasePipeline() const noexcept -> Pipeline const& {
    return BasePipeline;
}

auto PipelineGroup::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                     ParameterSpanVariant&& parameterSpan) -> ParameterSpanVariant& {
    if (!ParameterSpace.HasParameterSpanSet(artifactVariantPointer))
        ParameterSpace.AddParameterSpanSet(artifactVariantPointer);

    return ParameterSpace.AddParameterSpan(artifactVariantPointer, std::move(parameterSpan));
}

auto PipelineGroup::RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                        const ParameterSpanVariant& parameterSpan) -> void {
    ParameterSpace.RemoveParameterSpan(artifactVariantPointer, parameterSpan);
}
