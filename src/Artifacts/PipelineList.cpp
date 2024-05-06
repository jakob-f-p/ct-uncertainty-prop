#include "PipelineList.h"

#include "Pipeline.h"
#include "../Modeling/CtStructureTree.h"

PipelineList::PipelineList(CtStructureTree& structureTree) :
        StructureTree(structureTree) {

    structureTree.AddTreeEventCallback([&](CtStructureTreeEvent const& event) {
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
    return *Pipelines.at(idx);
}

auto PipelineList::AddPipeline() -> Pipeline& {
    return *Pipelines.emplace_back(std::make_unique<Pipeline>(StructureTree));
}

void PipelineList::RemovePipeline(Pipeline& pipeline) {
    auto removeIt = std::find_if(Pipelines.begin(), Pipelines.end(),
                                 [&](auto& p) { return p.get() == &pipeline; });
    if (removeIt == Pipelines.end())
        throw std::runtime_error("Given pipeline could not be removed because it was not present");

    Pipelines.erase(removeIt);
}

void PipelineList::AddPipelineEventCallback(PipelineEventCallback&& pipelineEventCallback) {
    PipelineEventCallbacks.emplace_back(std::move(pipelineEventCallback));
}

void PipelineList::ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) {
    for (auto& pipeline: Pipelines)
        pipeline->ProcessCtStructureTreeEvent(event);

    for (const auto& callback : PipelineEventCallbacks)
        callback();
}
