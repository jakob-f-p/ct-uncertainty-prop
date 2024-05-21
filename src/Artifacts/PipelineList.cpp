#include "PipelineList.h"

#include "Pipeline.h"
#include "../Modeling/CtStructureTree.h"

PipelineList::PipelineList(CtStructureTree& structureTree, CtDataSource& dataSource) :
        StructureTree(structureTree),
        DataSource(dataSource) {

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
    return *Pipelines.emplace_back(std::make_unique<Pipeline>(StructureTree, DataSource));
}

void PipelineList::RemovePipeline(Pipeline& pipeline) {
    auto removeIt = std::find_if(Pipelines.begin(), Pipelines.end(),
                                 [&](auto& p) { return p.get() == &pipeline; });
    if (removeIt == Pipelines.end())
        throw std::runtime_error("Given pipeline could not be removed because it was not present");

    EmitPipelineEvent({ PipelineEventType::PRE_REMOVE, &pipeline });

    Pipelines.erase(removeIt);
}

void PipelineList::AddTreeEventCallback(TreeEventCallback&& treeEventCallback) {
    TreeEventCallbacks.emplace_back(std::move(treeEventCallback));
}

void PipelineList::ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) {
    for (auto& pipeline: Pipelines)
        pipeline->ProcessCtStructureTreeEvent(event);

    for (auto const& callback : TreeEventCallbacks)
        callback();
}

void PipelineList::AddPipelineEventCallback(PipelineEventCallback&& pipelineEventCallback) {
    PipelineEventCallbacks.emplace_back(std::move(pipelineEventCallback));
}

auto PipelineList::EmitPipelineEvent(PipelineEvent event) const noexcept -> void {
    for (auto const& callback : PipelineEventCallbacks)
        callback(event);
}
