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

    [[nodiscard]] auto
    GetBatch() const -> PipelineBatch const&;

    using ProgressEventCallback = std::function<void(double)>;
    auto
    GenerateImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExportImages(uint32_t groupIdx, ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

private:
    friend class PipelineBatch;

    std::string Name;
    Pipeline const& BasePipeline;
    std::unique_ptr<PipelineParameterSpace> ParameterSpace;
    std::unique_ptr<PipelineBatch> Batch = nullptr;

    static uint16_t PipelineGroupId;
};
