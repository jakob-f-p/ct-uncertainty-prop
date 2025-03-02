#pragma once

#include "PipelineGroup.h"
#include "IO/HdfImageReadHandle.h"

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
    HdfImageReadHandle ImageHandle;
    std::vector<double> FeatureValues;
    std::vector<double> PcaCoordinates;
    std::vector<double> TsneCoordinates;
};

struct PipelineBatchData {
    PipelineGroup const& Group;
    std::vector<double> PcaExplainedVarianceRatios;
    Vector2DDouble PcaPrincipalAxes;
    std::vector<ParameterSpaceStateData> StateDataList;
};

struct PipelineBatchListData {
    using StateDataLists = std::vector<PipelineBatchData>;

    [[nodiscard]] auto
    TrimTo(uint16_t groupId) const -> PipelineBatchListData {
        PipelineBatchListData batchListData { *this };

        for (int i = 0; i < batchListData.Data.size(); i++) {
            auto& batchData = batchListData.Data.at(i);

            if (i != groupId) {
                batchData.PcaExplainedVarianceRatios.clear();
                batchData.PcaPrincipalAxes.clear();
                batchData.StateDataList.clear();
            }
        }

        return batchListData;
    }

    enum struct AnalysisType : uint8_t { PCA, TSNE };

    [[nodiscard]] auto
    TrimTo(QList<QPointF> const& pointsToKeep, AnalysisType analysisType) const -> PipelineBatchListData {
        PipelineBatchListData batchListData { GroupList, FeatureNames, {}, Time };

        for (auto const& batchData : Data) {
            PipelineBatchData trimmedBatchData { batchData.Group, {}, {}, {} };

            std::ranges::copy_if(batchData.StateDataList,
                                 std::back_inserter(trimmedBatchData.StateDataList),
                                 [&pointsToKeep, analysisType](ParameterSpaceStateData const& spaceStateData) {
                                     QPointF const point = [analysisType, &spaceStateData] {
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
    }

    [[nodiscard]] auto
    GetSpaceStateData(SampleId const& sampleId) -> ParameterSpaceStateData& {
        return Data.at(sampleId.GroupIdx).StateDataList.at(sampleId.StateIdx);
    }

    [[nodiscard]] auto
    GetSpaceStateData(SampleId const& sampleId) const -> ParameterSpaceStateData const& {
        return Data.at(sampleId.GroupIdx).StateDataList.at(sampleId.StateIdx);
    }

    [[nodiscard]] auto
    GetBatchWithPcaData() const -> PipelineBatchData const& {
        return *std::ranges::find_if(Data, [](PipelineBatchData const& data) {
            return !data.PcaExplainedVarianceRatios.empty() && !data.PcaPrincipalAxes.empty();
        });
    }

    struct MTimes {
        vtkMTimeType Image, Feature, Pca, Tsne;
        vtkMTimeType Total;

        [[nodiscard]] auto
        operator==(MTimes const& other) const noexcept -> bool = default;
    };

    PipelineGroupList const& GroupList;
    std::vector<std::string> const& FeatureNames;
    StateDataLists Data;
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
    Get(int idx) const noexcept -> PipelineGroup&;

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint16_t;

    using ProgressEventCallback = std::function<void(double)>;
    auto
    GenerateImages(ProgressEventCallback const& callback = [](double){}) const -> void;

    auto
    ExtractFeatures(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    DoPCAs(uint8_t numberOfDimensions, ProgressEventCallback const& callback = [](double) {}) const -> void;

    [[nodiscard]] static auto
    DoPCAForSubset(PipelineBatchListData const& subsetData,
                   ProgressEventCallback const& callback = [](double) {}) -> PipelineBatchListData;

    auto
    DoTsne(uint8_t numberOfDimensions, ProgressEventCallback const& callback = [](double) {}) const -> void;

    [[nodiscard]] auto
    GetDataStatus() const noexcept -> DataStatus;

    [[nodiscard]] auto
    GetBatchData() const noexcept -> std::optional<PipelineBatchListData>;

    auto
    ExportImagesHdf5(std::filesystem::path const& exportPath,
                     ProgressEventCallback const& callback = [](double) {}) const -> void;

    auto
    ExportImagesVtk(std::filesystem::path const& exportDir,
                    ProgressEventCallback const& callback = [](double) {}) const -> void;

    auto
    ImportImages(std::filesystem::path const& importFilePath,
                 ProgressEventCallback const& callback = [](double) {}) const -> void;

    auto
    ExportFeatures(std::filesystem::path const& exportPath,
                   ProgressEventCallback const& callback = [](double) {}) const -> void;

    auto
    ImportFeatures(std::filesystem::path const& importFilePath,
                   ProgressEventCallback const& callback = [](double) {}) const -> void;

    auto
    AddPipelineGroup(Pipeline const& pipeline, const std::string& name = "") -> PipelineGroup&;

    auto
    RemovePipelineGroup(PipelineGroup const& pipelineGroup) -> void;

    [[nodiscard]] auto
    FindPipelineGroupsByBasePipeline(Pipeline const& basePipeline) const noexcept -> std::vector<PipelineGroup const*>;

private:
    struct ProgressUpdater {
        auto
        operator()(double current) const noexcept -> void;

        int const Idx;
        std::vector<double>& ProgressList;
        ProgressEventCallback const& Callback;
    };

    struct WeightedProgressUpdater {
        auto
        operator()(double current) const noexcept -> void;

        int const Idx;
        std::vector<double>& ProgressList;
        std::vector<double> GroupSizeWeightVector;
        ProgressEventCallback const& Callback;
    };

    struct MultiTaskProgressUpdater {
        auto
        operator()(double current) const noexcept -> void;

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


    static std::filesystem::path const DataDirectory;

public:
    static std::filesystem::path const FeatureDirectory;
    static std::filesystem::path ImagesFile;
};
