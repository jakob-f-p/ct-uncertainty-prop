#include "PipelineList.h"

#include "Pipeline.h"
#include "../Modeling/CtStructureTree.h"
#include "../App.h"

PipelineList::PipelineList(CtStructureTree& structureTree) {
    structureTree.AddTreeEventCallback([&](const CtStructureTreeEvent& event) {
            ProcessCtStructureTreeEvent(event);
    });
}

auto PipelineList::IsEmpty() const noexcept -> bool {
    return Pipelines.empty();
}

auto PipelineList::GetSize() const noexcept -> int {
    return static_cast<int>(Pipelines.size());
}

auto PipelineList::Get(int idx) noexcept -> Pipeline& {
    return Pipelines.at(idx);
}

auto PipelineList::AddPipeline() noexcept -> Pipeline& {
    return Pipelines.emplace_back(App::GetInstance()->GetCtDataTree().StructureCount());
}

void PipelineList::RemovePipeline(Pipeline& pipeline) {
    auto removeIt = std::find(Pipelines.begin(), Pipelines.end(), pipeline);
    if (removeIt == Pipelines.end())
        throw std::runtime_error("Given pipeline could not be removed because it was not present");

    Pipelines.erase(removeIt);
}

auto PipelineList::NumberOfPipelines() const noexcept -> int {
    return static_cast<int>(Pipelines.size());
}

void PipelineList::AddPipelineEventCallback(PipelineEventCallback&& pipelineEventCallback) {
    PipelineEventCallbacks.emplace_back(pipelineEventCallback);
}

void PipelineList::ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) {
    for (auto& pipeline: Pipelines)
        pipeline.ProcessCtStructureTreeEvent(event);

    for (const auto& callback : PipelineEventCallbacks)
        callback();
}
