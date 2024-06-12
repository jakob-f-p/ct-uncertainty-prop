#pragma once

#include "../Artifacts/Pipeline.h"
#include "ArtifactVariantPointer.h"

#include <string>
#include <vector>

class PipelineBatch;
class PipelineParameterSpace;
class PipelineParameterSpan;
class PipelineParameterSpaceState;


class PipelineGroup {
public:
    explicit PipelineGroup(Pipeline const& basePipeline, std::string name = "");
    ~PipelineGroup();

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetBasePipeline() const noexcept -> Pipeline const&;

    [[nodiscard]] auto
    GetParameterSpace() noexcept -> PipelineParameterSpace&;

    [[nodiscard]] auto
    GetParameterSpace() const noexcept -> PipelineParameterSpace const&;

    using ProgressEventCallback = std::function<void(double)>;
    auto
    GenerateImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExportImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExtractFeatures(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

private:
    friend class PipelineBatch;

    std::string Name;
    uint16_t const GroupId;
    Pipeline const& BasePipeline;
    std::unique_ptr<PipelineParameterSpace> ParameterSpace;
    std::unique_ptr<PipelineBatch> Batch;

    static uint16_t PipelineGroupId;
};
