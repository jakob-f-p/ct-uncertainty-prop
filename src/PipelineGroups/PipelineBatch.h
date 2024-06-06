#pragma once

#include <vtkSmartPointer.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

class PipelineGroup;
class PipelineParameterSpaceState;

class vtkImageData;


class PipelineBatch {
    using ParameterSpaceState = std::unique_ptr<PipelineParameterSpaceState>;
    using PipelineStates = std::vector<ParameterSpaceState>;

public:
    explicit PipelineBatch(PipelineGroup const& pipelineGroup);

    struct ExportPathPair {
        std::filesystem::path Radiodensities;
        std::filesystem::path Mask;

        static std::filesystem::path const DataDirectory;
        static std::filesystem::path const InputDirectory;
        static std::filesystem::path const FeatureDirectory;
    };

    struct Image {
        PipelineParameterSpaceState const& State;
        vtkSmartPointer<vtkImageData> ImageData;
        std::optional<ExportPathPair> Paths;
    };

    using ProgressEventCallback = std::function<void(double)>;

    auto
    GenerateImages(ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExportImages(uint32_t groupIdx, ProgressEventCallback const& callback = [](double) {}) -> void;

    using ExportPathVector = std::vector<ExportPathPair>;

    auto
    ExtractFeatures() -> void;

private:
    [[nodiscard]] auto
    GetIdx(PipelineParameterSpaceState const& state) const -> uint32_t;

    using PipelineImages = std::vector<std::optional<Image>>;

    struct DoExport {
        auto
        operator()(std::optional<Image>& optionalImage) const -> void;

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
    PipelineImages Images;
};
