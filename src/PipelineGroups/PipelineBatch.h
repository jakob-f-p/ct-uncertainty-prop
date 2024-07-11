#pragma once

#include "Types.h"
#include "../Utils/TimeStampedData.h"

#include <vtkSmartPointer.h>

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <vector>

class PipelineGroup;
class PipelineParameterSpaceState;

class vtkImageData;

struct ParameterSpaceStateData;


struct PipelineImageData {
    PipelineParameterSpaceState const& State;
    vtkSmartPointer<vtkImageData> ImageData;
};

class PipelineBatch {
    using ParameterSpaceState = std::unique_ptr<PipelineParameterSpaceState>;
    using PipelineStates = std::vector<std::unique_ptr<PipelineParameterSpaceState>>;

public:
    explicit PipelineBatch(PipelineGroup const& pipelineGroup);

    struct ExportPathPair {
        std::filesystem::path Radiodensities;
        std::filesystem::path Mask;

        static std::filesystem::path const DataDirectory;
        static std::filesystem::path const InputDirectory;
        static std::filesystem::path const FeatureDirectory;
    };

    auto
    UpdateParameterSpaceStates() noexcept -> void;

    using BatchImages = std::vector<PipelineImageData>;

    using ProgressEventCallback = std::function<void(double)>;

    auto
    GenerateImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    using ExportPathVector = std::vector<ExportPathPair>;

    auto
    ExportImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExtractFeatures(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    DoPCA(uint8_t numberOfDimensions) -> void;

    [[nodiscard]] auto
    GetImageData() -> TimeStampedData<std::vector<PipelineImageData*>>;

    [[nodiscard]] auto
    GetFeatureData() const -> TimeStampedDataRef<FeatureData>;

    [[nodiscard]] auto
    GetPcaData() const -> TimeStampedDataRef<SampleCoordinateData>;

    [[nodiscard]] auto
    GetTsneData() const -> TimeStampedDataRef<SampleCoordinateData>;

    auto
    SetTsneData(SampleCoordinateData&& tsneData) -> void;

    [[nodiscard]] auto
    GetDataStatus() const noexcept -> DataStatus;

    auto
    ExportGeneratedImages(std::filesystem::path const& exportDir,
                          ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ImportImages(std::vector<std::filesystem::path> const& importFilePaths,
                 ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ImportFeatures(std::filesystem::path const& importFilePaths,
                   ProgressEventCallback const& callback = [](double) {}) -> void;

private:
    [[nodiscard]] auto
    GetIdx(PipelineParameterSpaceState const& state) const -> uint32_t;

    struct DoExport {
        auto
        operator()(PipelineImageData& image) const -> void;

        PipelineBatch& Batch;
        uint32_t GroupIdx;
        std::string TimeStampString;
        std::atomic<double>& Progress;
        std::atomic_uint& NumberOfExportedImages;
        size_t NumberOfImages;
    };

    struct DoExport1 {
        auto
        operator()(PipelineImageData& image) const -> void;

        PipelineBatch& Batch;
        uint32_t GroupIdx;
        std::string TimeStampString;
        std::atomic<double>& Progress;
        std::atomic_uint& NumberOfExportedImages;
        size_t NumberOfImages;
    };

    PipelineGroup const& Group;
    ParameterSpaceState InitialState;
    PipelineStates States;
    TimeStampedData<BatchImages> Images;
    TimeStampedData<ExportPathVector> ExportedImages;
    TimeStampedData<FeatureData> Features;
    TimeStampedData<SampleCoordinateData> PcaData;
    TimeStampedData<SampleCoordinateData> TsneData;
};
