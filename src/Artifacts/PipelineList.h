#pragma once

#include "Pipeline.h"

#include <functional>
#include <vector>

class CtDataSource;
class CtStructureTree;
struct CtStructureTreeEvent;


enum struct PipelineEventType : uint8_t {
    PRE_REMOVE,
    POST_ADD
};

struct PipelineEvent {
    PipelineEventType Type;
    Pipeline* PipelinePointer;
};

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
    Get(int idx) const noexcept -> Pipeline const&;

    auto
    AddPipeline() -> Pipeline&;

    auto
    RemovePipeline(Pipeline& pipeline) -> void;

    using TreeEventCallback = std::function<void()>;
    void AddTreeEventCallback(TreeEventCallback&& treeEventCallback);

    auto
    ProcessCtStructureTreeEvent(CtStructureTreeEvent const& event) const -> void;

    using PipelineEventCallback = std::function<void(PipelineEvent const&)>;
    void AddPipelineEventCallback(PipelineEventCallback&& pipelineEventCallback);

    auto
    EmitPipelineEvent(PipelineEvent event) const noexcept -> void;

private:
    CtStructureTree& StructureTree;
    std::vector<std::unique_ptr<Pipeline>> Pipelines;
    std::vector<TreeEventCallback> TreeEventCallbacks;
    std::vector<PipelineEventCallback> PipelineEventCallbacks;
};
