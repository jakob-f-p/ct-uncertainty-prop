#pragma once

#include "ArtifactVariantPointer.h"
#include "../Artifacts/Pipeline.h"
#include "../Utils/Types.h"

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
    GetImageData() -> std::vector<PipelineImageData*>;

    [[nodiscard]] auto
    GetFeatureData() const -> FeatureData const&;

    [[nodiscard]] auto
    GetPcaData() const -> SampleCoordinateData const&;

    [[nodiscard]] auto
    GetTsneData() const -> SampleCoordinateData const&;

    auto
    SetTsneData(SampleCoordinateData&& tsneData) -> void;

    [[nodiscard]] auto
    DataHasBeenGenerated() const noexcept -> bool;

    [[nodiscard]] auto
    GetDataMTime() const noexcept -> vtkMTimeType;

    auto
    AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan&;

    auto
    RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                        PipelineParameterSpan const& parameterSpan) -> void;

private:
    friend class PipelineBatch;

    std::string Name;
    uint16_t const GroupId;
    Pipeline const& BasePipeline;
    std::unique_ptr<PipelineParameterSpace> ParameterSpace;
    std::unique_ptr<PipelineBatch> Batch;

    static uint16_t PipelineGroupId;
};
