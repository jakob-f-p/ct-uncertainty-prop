#pragma once

#include "PipelineGroup.h"

#include <QList>
#include <QPointF>

#include <vtkSmartPointer.h>

#include <functional>
#include <vector>

class PipelineGroupList;
class PipelineList;
class PipelineParameterSpaceState;

class vtkImageData;


struct ParameterSpaceStateData {
    PipelineParameterSpaceState const& State;
    vtkSmartPointer<vtkImageData> ImageData;
    std::vector<double> FeatureValues;
    std::vector<double> PcaCoordinates;
    std::vector<double> TsneCoordinates;
};

struct PipelineBatchData {
    PipelineGroup const& Group;
    std::vector<ParameterSpaceStateData> StateDataList;
};

struct PipelineBatchListData {
    using StateDataLists = std::vector<PipelineBatchData>;

    [[nodiscard]] auto
    GetBatchData(uint16_t groupId) -> PipelineBatchData& { return Data.at(groupId); };

    [[nodiscard]] auto
    GetBatchData(uint16_t groupId) const -> PipelineBatchData const& { return Data.at(groupId); };

    [[nodiscard]] auto
    TrimTo(uint16_t groupId) const -> PipelineBatchListData {
        PipelineBatchListData batchListData { *this };

        for (int i = 0; i < batchListData.Data.size(); i++) {
            auto& batchData = batchListData.Data.at(i);

            if (i != groupId)
                batchData.StateDataList.clear();
        }

        return batchListData;
    };

    enum struct AnalysisType : uint8_t { PCA, TSNE };

    [[nodiscard]] auto
    TrimTo(QList<QPointF> const& pointsToKeep, AnalysisType analysisType) const -> PipelineBatchListData {
        PipelineBatchListData batchListData { GroupList, FeatureNames, {}, Time };

        for (auto const& batchData : Data) {
            PipelineBatchData trimmedBatchData { batchData.Group, {} };

            std::copy_if(batchData.StateDataList.begin(), batchData.StateDataList.end(),
                         std::back_inserter(trimmedBatchData.StateDataList),
                         [&pointsToKeep, analysisType](ParameterSpaceStateData const& spaceStateData) {
                QPointF const point = [analysisType, &spaceStateData]() {
                    switch (analysisType) {
                        case AnalysisType::PCA:  return QPointF { spaceStateData.PcaCoordinates.at(0),
                                                                  spaceStateData.PcaCoordinates.at(1) };
                        case AnalysisType::TSNE: return QPointF { spaceStateData.TsneCoordinates.at(0),
                                                                  spaceStateData.TsneCoordinates.at(1) };
                        default: throw std::runtime_error("invalid analysis type");
                    }
                }();

                return pointsToKeep.contains(point);
            });

            batchListData.Data.emplace_back(trimmedBatchData);
        }

        return batchListData;
    };

    [[nodiscard]] auto
    GetSpaceStateData(SampleId const& sampleId) -> ParameterSpaceStateData& {
        return Data.at(sampleId.GroupIdx).StateDataList.at(sampleId.StateIdx);
    };

    [[nodiscard]] auto
    GetSpaceStateData(SampleId const& sampleId) const -> ParameterSpaceStateData const& {
        return Data.at(sampleId.GroupIdx).StateDataList.at(sampleId.StateIdx);
    };

    PipelineGroupList const& GroupList;
    std::vector<std::string> const& FeatureNames;
    StateDataLists Data;
    struct MTimes {
        vtkMTimeType Image, Feature, Pca, Tsne;
        vtkMTimeType Total;

        [[nodiscard]] auto
        operator==(MTimes const& other) const noexcept -> bool = default;
    };
    MTimes Time;
};


class PipelineGroupList {
public:
    explicit PipelineGroupList(PipelineList const& pipelines);

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

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
    ExtractFeatures(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    DoPCAs(uint8_t numberOfDimensions, ProgressEventCallback const& callback = [](double) {}) -> void;

    [[nodiscard]] auto
    DoPCAForSubset(PipelineBatchListData const& subsetData,
                   ProgressEventCallback const& callback = [](double) {}) const -> PipelineBatchListData;

    auto
    DoTsne(uint8_t numberOfDimensions, ProgressEventCallback const& callback = [](double) {}) -> void;

    [[nodiscard]] auto
    GetDataStatus() const noexcept -> DataStatus;

    [[nodiscard]] auto
    GetBatchData() const noexcept -> std::optional<PipelineBatchListData>;

    auto
    ExportGeneratedImages(std::filesystem::path const& exportDir,
                          ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ImportImages(std::vector<std::filesystem::path> const& importFilePaths,
                 ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExportFeatures(std::filesystem::path const& exportDir,
                   ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ImportFeatures(std::vector<std::filesystem::path> const& importFilePaths,
                   ProgressEventCallback const& callback = [](double) {}) -> void;

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

    struct MultiTaskProgressUpdater {
        auto
        operator()(double current) noexcept -> void;

        int const CurrentTask;
        int const NumberOfTasks;
        int const Idx;
        std::vector<double>& ProgressList;
        ProgressEventCallback const& Callback;
    };

    vtkTimeStamp TimeStamp;
    std::string Name;
    std::vector<std::unique_ptr<PipelineGroup>> PipelineGroups;
    PipelineList const& Pipelines;
};
