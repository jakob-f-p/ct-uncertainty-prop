#pragma once

#include "ArtifactVariantPointer.h"
#include "PipelineParameterSpan.h"

#include <unordered_map>
#include <variant>

class ImageArtifact;
class StructureArtifact;


class PipelineParameterSpanSet {
public:
    explicit PipelineParameterSpanSet(ArtifactVariantPointer artifactVariantPointer);

    auto
    AddParameterSpan(PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(PipelineParameterSpan const& parameterSpan) -> void;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetArtifactPointer() const noexcept -> ArtifactVariantPointer;

    [[nodiscard]] auto
    GetSize() const noexcept -> uint16_t;

    [[nodiscard]] auto
    Get(uint16_t idx) -> PipelineParameterSpan&;

    [[nodiscard]] auto
    GetIdx(PipelineParameterSpan const& parameterSpan) const -> uint16_t;

    [[nodiscard]] auto
    operator== (PipelineParameterSpanSet const& other) const noexcept -> bool;

private:
    friend class PipelineParameterSpace;

    using ParameterSpanSet = std::vector<PipelineParameterSpan>;

    ArtifactVariantPointer ArtifactPointer;
    ParameterSpanSet ParameterSpans;
};


class PipelineParameterSpace {
public:
    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    AddParameterSpan(PipelineParameterSpanSet& spanSet,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

    auto
    RemoveParameterSpan(PipelineParameterSpanSet& spanSet,
                        PipelineParameterSpan const& parameterSpan) -> void;

    [[nodiscard]] auto
    GetNumberOfSpans() const noexcept -> uint16_t;

    [[nodiscard]] auto
    GetNumberOfSpanSets() const noexcept -> uint16_t;

    [[nodiscard]] auto
    GetSpanSet(uint16_t idx) -> PipelineParameterSpanSet&;

    [[nodiscard]] auto
    GetSpanSet(PipelineParameterSpan const& parameterSpan) -> PipelineParameterSpanSet&;

    [[nodiscard]] auto
    GetSpanSetIdx(PipelineParameterSpanSet const& spanSet) const -> uint16_t;

    [[nodiscard]] auto
    GetSpanSetName(PipelineParameterSpanSet const& spanSet) const -> std::string;

private:
    friend class PipelineParameterSpaceModel;

    [[nodiscard]] auto
    ContainsSetForArtifactPointer(ArtifactVariantPointer artifactVariantPointer) const noexcept -> bool;

    [[nodiscard]] auto
    GetSetForArtifactPointer(ArtifactVariantPointer artifactVariantPointer) -> PipelineParameterSpanSet&;

    std::vector<PipelineParameterSpanSet> ParameterSpanSets;
};
