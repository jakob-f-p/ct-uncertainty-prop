#pragma once

#include "PipelineGroup.h"

#include <functional>
#include <vector>

class PipelineGroupList {
public:
    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    IsEmpty() const noexcept -> bool;

    [[nodiscard]] auto
    GetSize() const noexcept -> int;

    [[nodiscard]] auto
    Get(int idx) noexcept -> PipelineGroup&;

    auto
    AddPipelineGroup(Pipeline const& pipeline) -> PipelineGroup&;

    auto
    RemovePipelineGroup(PipelineGroup const& pipelineGroup) -> void;

    [[nodiscard]] auto
    FindPipelineGroupsByBasePipeline(Pipeline const& basePipeline) const noexcept -> std::vector<PipelineGroup const*>;

private:
    std::string Name;
    std::vector<std::unique_ptr<PipelineGroup>> PipelineGroups;
};
