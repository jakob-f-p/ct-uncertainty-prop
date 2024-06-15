#include "PipelineGroupList.h"

#include "PipelineBatch.h"
#include "PipelineGroup.h"
#include "PipelineParameterSpace.h"
#include "../Artifacts/PipelineList.h"
#include "../Modeling/CtStructureTree.h"
#include "../Utils/PythonInterpreter.h"
#include "../App.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <ranges>


PipelineGroupList::PipelineGroupList(const PipelineList& pipelines) :
        Pipelines(pipelines) {}

auto PipelineGroupList::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroupList::GetSize() const noexcept -> uint8_t {
    return PipelineGroups.size();
}

auto PipelineGroupList::GetBasePipelines() const noexcept -> std::vector<Pipeline const*> {
    std::vector<Pipeline const*> basePipelines;

    for (int i = 0; i < Pipelines.GetSize(); i++)
        basePipelines.emplace_back(&Pipelines.Get(i));

    return basePipelines;
}

auto PipelineGroupList::Get(int idx) noexcept -> PipelineGroup& {
    return *PipelineGroups.at(idx);
}

auto PipelineGroupList::GetNumberOfPipelines() const noexcept -> uint16_t {
    return std::transform_reduce(PipelineGroups.cbegin(), PipelineGroups.cend(), 0, std::plus{},
                                 [](auto const& group) { return group->GetParameterSpace().GetNumberOfPipelines(); });
}

auto PipelineGroupList::AddPipelineGroup(Pipeline const& pipeline, std::string const& name) -> PipelineGroup& {
    return *PipelineGroups.emplace_back(std::make_unique<PipelineGroup>(pipeline, name));
}

void PipelineGroupList::RemovePipelineGroup(PipelineGroup const& pipeline) {
    auto removeIt = std::find_if(PipelineGroups.begin(), PipelineGroups.end(),
                                 [&](auto& p) { return p.get() == &pipeline; });
    if (removeIt == PipelineGroups.end())
        throw std::runtime_error("Given pipeline group could not be removed because it was not present");

    PipelineGroups.erase(removeIt);
}

auto PipelineGroupList::FindPipelineGroupsByBasePipeline(Pipeline const& basePipeline) const noexcept
        -> std::vector<PipelineGroup const*> {
    auto filteredPipelineGroups = PipelineGroups
            | std::views::filter([&](auto& group) { return group->GetBasePipeline() == basePipeline; })
            | std::views::transform([](auto& group) { return group.get(); })
            | std::views::common;

    return { filteredPipelineGroups.begin(), filteredPipelineGroups.end() };
}

auto PipelineGroupList::GenerateImages(ProgressEventCallback const& callback) -> void {
    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    callback(0.0);

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->GenerateImages(ProgressUpdater { i, progressList, callback });
}

auto
PipelineGroupList::ExportImages(PipelineGroupList::ProgressEventCallback const& callback) -> void {
    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    callback(0.0);

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->ExportImages(ProgressUpdater { i, progressList, callback });
}

auto PipelineGroupList::ExtractFeatures(PipelineGroupList::ProgressEventCallback const& callback) -> void {
    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    std::vector<int> groupSizeVector (PipelineGroups.size(), 0.0);
    std::vector<double> groupSizeWeightVector (PipelineGroups.size(), 0.0);
    std::transform(PipelineGroups.begin(), PipelineGroups.end(), groupSizeVector.begin(),
                   [](auto const& group) { return group->GetParameterSpace().GetNumberOfPipelines(); });
    int const totalNumberOfPipelines = std::reduce(groupSizeVector.cbegin(), groupSizeVector.cend());
    std::transform(groupSizeVector.cbegin(), groupSizeVector.cend(), groupSizeWeightVector.begin(),
                   [=](int size) { return static_cast<double>(size) / static_cast<double>(totalNumberOfPipelines); });

    callback(0.0);

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->ExtractFeatures(WeightedProgressUpdater { i, progressList,
                                                                     groupSizeWeightVector, callback });
}

auto PipelineGroupList::DoPCAs(uint8_t numberOfDimensions, ProgressEventCallback const& callback) -> void {
    for (int i = 0; i < PipelineGroups.size(); i++) {
        double const progress = static_cast<double>(PipelineGroups.size()) / static_cast<double>(i);
        callback(progress);

        PipelineGroups[i]->DoPCA(numberOfDimensions);
    }

    callback(1.0);
}

auto PipelineGroupList::DoTsne(uint8_t numberOfDimensions, ProgressEventCallback const& callback) -> void {
    callback(0.0);

    std::vector<FeatureData const*> featureDataVector;
    for (auto& group: PipelineGroups)
        featureDataVector.emplace_back(&group->GetFeatureData());

    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    pybind11::object const tsneGroupCoordinateObject = interpreter.ExecuteFunction("tsne", "calculate",
                                                                                   featureDataVector,
                                                                                   numberOfDimensions);

    auto tsneGroupCoordinateData = tsneGroupCoordinateObject.cast<GroupCoordinateData>();

    assert(PipelineGroups.size() == tsneGroupCoordinateData.size());

    for (int i = 0; i < PipelineGroups.size(); i++) {
        auto& group = PipelineGroups[i];
        group->SetTsneData(std::move(tsneGroupCoordinateData[i]));
    }

    callback(1.0);
}

auto PipelineGroupList::GetBatchData() const noexcept -> std::optional<PipelineBatchListData> {
    bool const dataHasBeenGenerated = std::all_of(PipelineGroups.cbegin(), PipelineGroups.cend(),
                                            [](auto const& group) { return group->DataHasBeenGenerated(); });
    if (PipelineGroups.empty() || !dataHasBeenGenerated)
        return std::nullopt;

    std::vector<std::vector<PipelineImageData*>> imageDataVectors;
    std::vector<FeatureData const*> featureDataVector;
    std::vector<SampleCoordinateData const*> pcaDataVector;
    std::vector<SampleCoordinateData const*> tsneDataVector;
    std::vector<vtkMTimeType> mTimes;

    for (auto const& group: PipelineGroups) {
        imageDataVectors.emplace_back(group->GetImageData());
        featureDataVector.emplace_back(&group->GetFeatureData());
        pcaDataVector.emplace_back(&group->GetPcaData());
        tsneDataVector.emplace_back(&group->GetTsneData());
        mTimes.emplace_back(group->GetDataMTime());
    }

    std::vector<std::string> const& featureNames = featureDataVector[0]->Names;

    PipelineBatchListData::StateDataLists stateDataLists;
    stateDataLists.reserve(PipelineGroups.size());

    for (int i = 0; i < imageDataVectors.size(); i++) {
        std::vector<PipelineImageData*>& imageDataVector = imageDataVectors[i];
        FeatureData const& featureData = *featureDataVector[i];
        SampleCoordinateData const& pcaData = *pcaDataVector[i];
        SampleCoordinateData const& tsneData = *tsneDataVector[i];

        std::vector<ParameterSpaceStateData> stateDataList;
        stateDataList.reserve(imageDataVectors[0].size());
        for (int j = 0; j < imageDataVectors[0].size(); j++)
            stateDataList.emplace_back(imageDataVector[j]->State, imageDataVector[j]->ImageData,
                                       featureData.Values[j], pcaData[j], tsneData[j]);

        stateDataLists.emplace_back(*PipelineGroups[i], std::move(stateDataList));
    }

    vtkMTimeType const mTime = *std::min_element(mTimes.cbegin(), mTimes.cend());

    return PipelineBatchListData { featureNames, stateDataLists, mTime };
}

auto PipelineGroupList::ProgressUpdater::operator()(double current) noexcept -> void {
    ProgressList[Idx] = current;

    double const totalProgress = std::reduce(ProgressList.cbegin(), ProgressList.cend())
                                        / static_cast<double>(ProgressList.size());
    Callback(totalProgress);
}

auto PipelineGroupList::WeightedProgressUpdater::operator()(double current) noexcept -> void {
    ProgressList[Idx] = current;

    double const totalProgress = std::transform_reduce(ProgressList.cbegin(), ProgressList.cend(),
                                                       GroupSizeWeightVector.cbegin(),
                                                       0.0, std::plus{}, std::multiplies{});
    Callback(totalProgress);
}
