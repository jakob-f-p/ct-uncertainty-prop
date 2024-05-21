#include "PipelineGroupList.h"

#include "PipelineGroup.h"
#include "../Modeling/CtStructureTree.h"

#include <ranges>

auto PipelineGroupList::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroupList::IsEmpty() const noexcept -> bool {
    return PipelineGroups.empty();
}

auto PipelineGroupList::GetSize() const noexcept -> int {
    return static_cast<int>(PipelineGroups.size());
}

auto PipelineGroupList::Get(int idx) noexcept -> PipelineGroup& {
    return *PipelineGroups.at(idx);
}

auto PipelineGroupList::AddPipelineGroup(Pipeline const& pipeline) -> PipelineGroup& {
    return *PipelineGroups.emplace_back(std::make_unique<PipelineGroup>(pipeline));
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
            | std::views::filter([&](auto& group) { return group->GetBasePipeline() == basePipeline; });

    std::vector<PipelineGroup const*> pipelineGroups;
    for (auto& group : filteredPipelineGroups)
        pipelineGroups.emplace_back(group.get());

    return pipelineGroups;
}
