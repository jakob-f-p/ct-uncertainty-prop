#pragma once

#include "ArtifactVariantPointer.h"
#include "Types.h"
#include "../Artifacts/Pipeline.h"
#include "../Utils/TimeStampedData.h"

#include <string>
#include <vector>

class PipelineBatch;
class PipelineParameterSpace;
class PipelineParameterSpan;
class PipelineParameterSpaceState;

struct PipelineImageData;


class PipelineGroup {
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

    using ProgressEventCallback = std::function<void(double)>;
    auto
    GenerateImages(ProgressEventCallback const& callback = [](double) {}) -> void;

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
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

private:
    friend class PipelineBatch;

    vtkTimeStamp TimeStamp;
    std::string Name;
    uint16_t const GroupId;
    Pipeline const& BasePipeline;
    std::unique_ptr<PipelineParameterSpace> ParameterSpace;
    std::unique_ptr<PipelineBatch> Batch;

    static uint16_t PipelineGroupId;
};
