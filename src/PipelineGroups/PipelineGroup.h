#pragma once

#include "ArtifactVariantPointer.h"
#include "Types.h"
#include "IO/HdfImageReadHandle.h"
#include "../Utils/TimeStampedData.h"

#include <string>
#include <vector>

class HdfImageReader;
class HdfImageWriter;
class Pipeline;
class PipelineParameterSpace;
class PipelineParameterSpan;
class PipelineParameterSpaceState;


class PipelineGroup {
    using HdfImageReadHandles = std::vector<HdfImageReadHandle>;

public:
    explicit PipelineGroup(Pipeline const& basePipeline, std::string name = "");
    ~PipelineGroup();

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

    [[nodiscard]] auto
    GetBasePipeline() const noexcept -> Pipeline const&;

    [[nodiscard]] auto
    GetParameterSpace() noexcept -> PipelineParameterSpace&;

    [[nodiscard]] auto
    GetParameterSpace() const noexcept -> PipelineParameterSpace const&;

    auto
    UpdateParameterSpaceStates() noexcept -> void;

    using ProgressEventCallback = std::function<void(double)>;
    auto
    GenerateImages(HdfImageWriter& imageWriter, ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    ExtractFeatures(HdfImageReader& imageReader, ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    DoPCA(uint8_t numberOfDimensions) -> void;

    [[nodiscard]] auto
    GetParameterSpaceStates() -> std::vector<std::reference_wrapper<PipelineParameterSpaceState>>;

    [[nodiscard]] auto
    GetImageData() const -> TimeStampedDataRef<HdfImageReadHandles>;

    [[nodiscard]] auto
    GetFeatureData() const -> TimeStampedDataRef<FeatureData>;

    [[nodiscard]] auto
    GetPcaData() const -> TimeStampedDataRef<PcaData>;

    [[nodiscard]] auto
    GetTsneData() const -> TimeStampedDataRef<SampleCoordinateData>;

    auto
    SetFeatureData(FeatureData&& featureData) -> void;

    auto
    SetTsneData(SampleCoordinateData&& tsneData) -> void;

    [[nodiscard]] auto
    GetDataStatus() const noexcept -> DataStatus;

    auto
    ImportImages() -> void;

    auto
    ExportImagesVtk(std::filesystem::path const& exportDir,
                    ProgressEventCallback const& callback = [](double) {}) -> void;

    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

    struct ImageMaskRefPair {
        SampleId Id;
        std::reference_wrapper<vtkImageData> Image;
        std::reference_wrapper<vtkImageData> Mask;
    };

private:
    friend class PipelineBatch;

    [[nodiscard]] static auto
    GetMaxImageBatchSize() -> uint64_t;

    using SpaceState = std::unique_ptr<PipelineParameterSpaceState>;
    using SpaceStates = std::vector<SpaceState>;

    std::string Name;
    uint16_t const GroupId;
    Pipeline const& BasePipeline;
    std::unique_ptr<PipelineParameterSpace> ParameterSpace;

    struct GroupData {
        SpaceState InitialState;
        SpaceStates States;

        TimeStampedData<HdfImageReadHandles> Images;
        TimeStampedData<FeatureData> Features;
        TimeStampedData<PcaData> PcaData;
        TimeStampedData<SampleCoordinateData> TsneData;
    };
    GroupData Data;

    static uint16_t PipelineGroupId;
};
