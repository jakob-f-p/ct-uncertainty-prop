#pragma once

#include "ArtifactVariantPointer.h"
#include "PipelineParameterSpan.h"

#include <vtkTimeStamp.h>

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
    Get(uint16_t idx) const -> PipelineParameterSpan const&;

    [[nodiscard]] auto
    GetIdx(PipelineParameterSpan const& parameterSpan) const -> uint16_t;

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint16_t;

    [[nodiscard]] auto
    operator== (PipelineParameterSpanSet const& other) const noexcept -> bool;

private:
    friend class PipelineParameterSpace;
    friend class ParameterSpanSetState;

    using ParameterSpanSet = std::vector<PipelineParameterSpan>;

    ArtifactVariantPointer ArtifactPointer;
    ParameterSpanSet ParameterSpans;
};


class PipelineParameterSpace {
public:
    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

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
    GetNumberOfPipelines() const noexcept -> uint16_t;

    [[nodiscard]] auto
    GetSpanSet(uint16_t idx) -> PipelineParameterSpanSet&;

    [[nodiscard]] auto
    GetSpanSet(uint16_t idx) const -> PipelineParameterSpanSet const&;

    [[nodiscard]] auto
    GetSpanSet(PipelineParameterSpan const& parameterSpan) -> PipelineParameterSpanSet&;

    [[nodiscard]] auto
    GetSpanSetIdx(PipelineParameterSpanSet const& spanSet) const -> uint16_t;

    [[nodiscard]] auto
    GetSpanSetName(PipelineParameterSpanSet const& spanSet) const -> std::string;

    [[nodiscard]] auto
    GenerateSpaceStates() -> std::vector<PipelineParameterSpaceState>;

private:
    friend class PipelineParameterSpaceModel;
    friend class PipelineParameterSpaceState;

    [[nodiscard]] auto
    ContainsSetForArtifactPointer(ArtifactVariantPointer artifactVariantPointer) const noexcept -> bool;

    [[nodiscard]] auto
    GetSetForArtifactPointer(ArtifactVariantPointer artifactVariantPointer) -> PipelineParameterSpanSet&;

    auto
    GenerateSpaceStatesRecursive(std::vector<PipelineParameterSpan*> const& spans,
                                 std::vector<PipelineParameterSpaceState>& states,
                                 int depth) -> void;

    std::vector<PipelineParameterSpanSet> ParameterSpanSets;
    vtkTimeStamp MTime;
};
