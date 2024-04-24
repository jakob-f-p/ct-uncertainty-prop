#pragma once

#include "Pipeline.h"

#include <functional>
#include <vector>

struct CtStructureTree;
struct CtStructureTreeEvent;

class PipelineList {
public:
    explicit PipelineList(CtStructureTree& structureTree);

    [[nodiscard]] auto
    IsEmpty() const noexcept -> bool;

    [[nodiscard]] auto
    GetSize() const noexcept -> int;

    [[nodiscard]] auto
    Get(int idx) noexcept -> Pipeline&;

    [[nodiscard]] auto
    NumberOfPipelines() const noexcept -> int;

    auto
    AddPipeline() -> Pipeline&;

    auto
    RemovePipeline(Pipeline& pipeline) -> void;

    using PipelineEventCallback = std::function<void()>;
    void AddPipelineEventCallback(PipelineEventCallback&& pipelineEventCallback);

    auto
    ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) -> void;

private:
    std::vector<std::unique_ptr<Pipeline>> Pipelines;
    std::vector<PipelineEventCallback> PipelineEventCallbacks;
};
