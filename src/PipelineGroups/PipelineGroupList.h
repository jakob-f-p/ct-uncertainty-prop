#pragma once

#include "PipelineGroup.h"

#include <vtkSmartPointer.h>

#include <functional>
#include <vector>

class PipelineList;
class PipelineParameterSpaceState;

class vtkImageData;


struct ParameterSpaceStateData {
    PipelineParameterSpaceState const& State;
    vtkSmartPointer<vtkImageData> ImageData;
    std::vector<double> const& FeatureValues;
    std::vector<double> const& PcaCoordinates;
    std::vector<double> const& TsneCoordinates;
};

struct PipelineBatchData {
    PipelineGroup const& Group;
    std::vector<ParameterSpaceStateData> StateDataList;
};

struct PipelineBatchListData {
    using StateDataLists = std::vector<PipelineBatchData>;

    [[nodiscard]] auto
    GetBatchData(uint16_t groupId) -> PipelineBatchData& {
        return Data.at(groupId);
    };

    [[nodiscard]] auto
    GetBatchData(uint16_t groupId) const -> PipelineBatchData const& {
        return Data.at(groupId);
    };

    [[nodiscard]] auto
    GetSpaceStateData(SampleId const& sampleId) -> ParameterSpaceStateData& {
        return Data.at(sampleId.GroupIdx).StateDataList.at(sampleId.StateIdx);
    };

    [[nodiscard]] auto
    GetSpaceStateData(SampleId const& sampleId) const -> ParameterSpaceStateData const& {
        return Data.at(sampleId.GroupIdx).StateDataList.at(sampleId.StateIdx);
    };

    std::vector<std::string> const& FeatureNames;
    StateDataLists Data;
    vtkMTimeType MTime;
};


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
    ExtractFeatures(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    DoPCAs(uint8_t numberOfDimensions, ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    DoTsne(uint8_t numberOfDimensions, ProgressEventCallback const& callback = [](double) {}) -> void;

    [[nodiscard]] auto
    GetBatchData() const noexcept -> std::optional<PipelineBatchListData>;

    auto
    AddPipelineGroup(Pipeline const& pipeline, const std::string& name = "") -> PipelineGroup&;

    auto
    RemovePipelineGroup(PipelineGroup const& pipelineGroup) -> void;

    [[nodiscard]] auto
    FindPipelineGroupsByBasePipeline(Pipeline const& basePipeline) const noexcept -> std::vector<PipelineGroup const*>;

private:
    struct ProgressUpdater {
        auto
        operator()(double current) noexcept -> void;

        int const Idx;
        std::vector<double>& ProgressList;
        ProgressEventCallback const& Callback;
    };

    struct WeightedProgressUpdater {
        auto
        operator()(double current) noexcept -> void;

        int const Idx;
        std::vector<double>& ProgressList;
        std::vector<double> GroupSizeWeightVector;
        ProgressEventCallback const& Callback;
    };

    std::string Name;
    std::vector<std::unique_ptr<PipelineGroup>> PipelineGroups;
    PipelineList const& Pipelines;
};
