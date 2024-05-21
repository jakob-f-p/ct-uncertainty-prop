#pragma once

#include "PipelineParameterSpan.h"

#include <unordered_map>
#include <variant>

class ImageArtifact;
class StructureArtifact;

using ParameterSpanVariant = std::variant<ParameterSpan<float>, ParameterSpan<FloatPoint>>;

class PipelineParameterSpanSet {
public:
    auto
    AddParameterSpan(ParameterSpanVariant&& spanVariant) -> ParameterSpanVariant&;

    auto
    RemoveParameterSpan(ParameterSpanVariant const& spanVariant) -> void;

private:
    friend class PipelineParameterSpanSet;

    using ParameterSpanSet = std::vector<ParameterSpanVariant>;

    ParameterSpanSet ParameterSpans;
};


using ArtifactVariantPointer = std::variant<ImageArtifact*, StructureArtifact*>;

class PipelineParameterSpace {
public:
    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     ParameterSpanVariant&& parameterSpan) -> ParameterSpanVariant&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        ParameterSpanVariant const& parameterSpan) -> void;

    [[nodiscard]] auto
    HasParameterSpanSet(ArtifactVariantPointer artifactVariantPointer) const noexcept -> bool;

    auto
    AddParameterSpanSet(ArtifactVariantPointer artifactVariantPointer) -> PipelineParameterSpanSet&;

    auto
    RemoveParameterSpanSet(ArtifactVariantPointer artifactVariantPointer) -> void;

private:
    using ArtifactParameterSpansMap = std::unordered_map<ArtifactVariantPointer, PipelineParameterSpanSet>;

    ArtifactParameterSpansMap ArtifactParametersMap;
};
