#pragma once

#include "PipelineGroup.h"

#include <functional>
#include <vector>

class PipelineList;

class PipelineGroupList {
public:
    explicit PipelineGroupList(PipelineList const& pipelines);

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetBasePipelines() const noexcept -> std::vector<Pipeline const*>;

    [[nodiscard]] auto
    GetSize() const noexcept -> uint8_t;

    [[nodiscard]] auto
    Get(int idx) noexcept -> PipelineGroup&;

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint16_t;

    using ProgressEventCallback = std::function<void(double)>;
    auto
    GenerateImages(ProgressEventCallback const& callback = [](double){}) -> void;

    auto
    ExportImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    AddPipelineGroup(Pipeline const& pipeline, const std::string& name = "") -> PipelineGroup&;

    auto
    RemovePipelineGroup(PipelineGroup const& pipelineGroup) -> void;

    [[nodiscard]] auto
    FindPipelineGroupsByBasePipeline(Pipeline const& basePipeline) const noexcept -> std::vector<PipelineGroup const*>;

private:
    std::string Name;
    std::vector<std::unique_ptr<PipelineGroup>> PipelineGroups;
    PipelineList const& Pipelines;
};
