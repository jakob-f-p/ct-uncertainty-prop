#pragma once

#include <vtkSmartPointer.h>

#include <memory>
#include <vector>

class PipelineGroup;
class PipelineParameterSpaceState;

class vtkImageData;


class PipelineImage {
public:
    explicit PipelineImage(PipelineParameterSpaceState const& state, PipelineGroup const& pipelineGroup);

private:
    PipelineParameterSpaceState const& State;
    vtkSmartPointer<vtkImageData> ImageData;
};


class PipelineBatch {
    using State = std::unique_ptr<PipelineParameterSpaceState>;
    using PipelineStates = std::vector<State>;
    using PipelineImages = std::vector<PipelineImage>;

public:
    explicit PipelineBatch(PipelineGroup const& pipelineGroup);

    auto
    GenerateImages() -> void;

    using ProgressEventCallback = std::function<void(double)>;
    auto
    AddProgressEventCallback(ProgressEventCallback&& callback) -> void;


private:
    PipelineGroup const& Group;
    State InitialState;
    PipelineStates States;

    using ProgressEventCallbacks = std::vector<ProgressEventCallback>;

    PipelineImages Images;
    ProgressEventCallbacks ProgressCallbacks;
};
