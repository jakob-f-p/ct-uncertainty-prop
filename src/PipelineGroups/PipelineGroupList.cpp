#include "PipelineGroupList.h"

#include "PipelineGroup.h"
#include "../Artifacts/PipelineList.h"
#include "../Modeling/CtStructureTree.h"

#include <ranges>

PipelineGroupList::PipelineGroupList(const PipelineList& pipelines) :
        Pipelines(pipelines) {}

auto PipelineGroupList::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroupList::GetSize() const noexcept -> uint8_t {
    return PipelineGroups.size();
}

auto PipelineGroupList::GetBasePipelines() const noexcept -> std::vector<Pipeline const*> {
    std::vector<Pipeline const*> basePipelines;

    for (int i = 0; i < Pipelines.GetSize(); i++)
        basePipelines.emplace_back(&Pipelines.Get(i));

    return basePipelines;
}

auto PipelineGroupList::Get(int idx) noexcept -> PipelineGroup& {
    return *PipelineGroups.at(idx);
}

auto PipelineGroupList::GetNumberOfPipelines() const noexcept -> uint16_t {
    return std::transform_reduce(PipelineGroups.cbegin(), PipelineGroups.cend(), 0, std::plus{},
                                 [](auto const& group) { return group->GetParameterSpace().GetNumberOfPipelines(); });
}

auto PipelineGroupList::AddPipelineGroup(Pipeline const& pipeline, std::string const& name) -> PipelineGroup& {
    return *PipelineGroups.emplace_back(std::make_unique<PipelineGroup>(pipeline, name));
}

void PipelineGroupList::RemovePipelineGroup(PipelineGroup const& pipeline) {
    auto removeIt = std::find_if(PipelineGroups.begin(), PipelineGroups.end(),
                                 [&](auto& p) { return p.get() == &pipeline; });
    if (removeIt == PipelineGroups.end())
        throw std::runtime_error("Given pipeline group could not be removed because it was not present");

    PipelineGroups.erase(removeIt);
}

auto PipelineGroupList::FindPipelineGroupsByBasePipeline(Pipeline const& basePipeline) const noexcept
        -> std::vector<PipelineGroup const*> {
    auto filteredPipelineGroups = PipelineGroups
            | std::views::filter([&](auto& group) { return group->GetBasePipeline() == basePipeline; })
            | std::views::transform([](auto& group) { return group.get(); })
            | std::views::common;

    return { filteredPipelineGroups.begin(), filteredPipelineGroups.end() };
}
