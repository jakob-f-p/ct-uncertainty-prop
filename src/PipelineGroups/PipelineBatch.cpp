#include "PipelineBatch.h"

#include "PipelineGroup.h"
#include "PipelineParameterSpace.h"
#include "PipelineParameterSpaceState.h"

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>


PipelineImage::PipelineImage(PipelineParameterSpaceState const& state, PipelineGroup const& pipelineGroup) :
        State(state),
        ImageData(pipelineGroup.GetBasePipeline().GetImageAlgorithm().GetOutput()) {}

PipelineBatch::PipelineBatch(PipelineGroup const& pipelineGroup) :
        Group(pipelineGroup),
        InitialState(new PipelineParameterSpaceState(*pipelineGroup.ParameterSpace)),
        States([&pipelineGroup]() {
            PipelineStates pipelineStates;
            pipelineStates.reserve(pipelineGroup.ParameterSpace->GetNumberOfPipelines());

            auto spaceStates = pipelineGroup.ParameterSpace->GenerateSpaceStates();

            for (auto& state : spaceStates)
                pipelineStates.emplace_back(std::make_unique<PipelineParameterSpaceState>(std::move(state)));

            return pipelineStates;
        }()) {}

auto PipelineBatch::GenerateImages() -> void {
    auto numberOfStates = States.size();

    for (int i = 0; i < numberOfStates; i++) {

        double progress = i / numberOfStates;
        for (auto const& callback : ProgressCallbacks)
            callback(progress);

        auto& state = States[i];
        if (state)
            throw std::runtime_error("State must not be nullptr");

        Images.emplace_back(*state, Group);
    }
}

auto PipelineBatch::AddProgressEventCallback(PipelineBatch::ProgressEventCallback&& callback) -> void {
    ProgressCallbacks.emplace_back(callback);
}
