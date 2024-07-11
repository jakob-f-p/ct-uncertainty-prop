#include "PipelineGroupList.h"

#include "PipelineBatch.h"
#include "PipelineGroup.h"
#include "PipelineParameterSpace.h"
#include "IO/HdfImageWriter.h"
#include "IO/HdfImageReader.h"
#include "../Artifacts/PipelineList.h"
#include "../Modeling/CtStructureTree.h"
#include "../Utils/PythonInterpreter.h"
#include "../App.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <ranges>
#include <regex>


PipelineGroupList::PipelineGroupList(const PipelineList& pipelines) :
        Pipelines(pipelines) {}

auto PipelineGroupList::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroupList::GetMTime() const noexcept -> vtkMTimeType {
    auto basePipelines = GetBasePipelines();
    std::vector<vtkMTimeType> basePipelineMTimes;
    basePipelineMTimes.reserve(basePipelines.size());
    std::transform(basePipelines.cbegin(), basePipelines.cend(), std::back_inserter(basePipelineMTimes),
                   [](auto const* pipeline) { return pipeline->GetMTime(); });
    vtkMTimeType const basePipelineTime = *std::max_element(basePipelineMTimes.cbegin(), basePipelineMTimes.cend());

    std::vector<vtkMTimeType> pipelineGroupMTimes;
    pipelineGroupMTimes.reserve(PipelineGroups.size());
    for (auto const& group : PipelineGroups)
        pipelineGroupMTimes.emplace_back(group->GetMTime());
    vtkMTimeType const groupMTime = *std::max_element(pipelineGroupMTimes.cbegin(), pipelineGroupMTimes.cend());

    return std::max({ TimeStamp.GetMTime(), basePipelineTime, groupMTime });
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
    TimeStamp.Modified();

    return *PipelineGroups.emplace_back(std::make_unique<PipelineGroup>(pipeline, name));
}

void PipelineGroupList::RemovePipelineGroup(PipelineGroup const& pipeline) {
    auto removeIt = std::find_if(PipelineGroups.begin(), PipelineGroups.end(),
                                 [&](auto& p) { return p.get() == &pipeline; });
    if (removeIt == PipelineGroups.end())
        throw std::runtime_error("Given pipeline group could not be removed because it was not present");

    TimeStamp.Modified();

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

    auto const now = round<std::chrono::seconds>(std::chrono::system_clock::now());
    std::string const timeStampString = std::format("{0:%Y}-{0:%m}-{0:%d}_{0:%H}-{0:%M}-{0:2%S}", now);

    ImagesFile = std::filesystem::path(DataDirectory) /= { std::format("images_{}.h5", timeStampString) };

    vtkNew<HdfImageWriter> imageWriter;
    imageWriter->SetFilename(ImagesFile);
    imageWriter->SetArrayNames({ "Radiodensities", "Segmentation Mask" });
    imageWriter->SetTotalNumberOfImages(GetNumberOfPipelines());

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->GenerateImages(*imageWriter, ProgressUpdater { i, progressList, callback });
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

    vtkNew<HdfImageReader> imageReader;
    imageReader->SetFilename(ImagesFile);
    imageReader->SetArrayNames({ "Radiodensities", "Segmentation Mask" });

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->ExtractFeatures(*imageReader, WeightedProgressUpdater { i, progressList,
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

auto PipelineGroupList::DoPCAForSubset(PipelineBatchListData const& subsetData,
                                       ProgressEventCallback const& callback) -> PipelineBatchListData {
    PipelineBatchListData batchListData { subsetData };

    callback(0.1);

    FeatureData featureData { subsetData.FeatureNames, {} };
    for (auto const& batchData : batchListData.Data)
        for (auto const& stateData : batchData.StateDataList)
            featureData.Values.push_back(stateData.FeatureValues);

    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    auto it = std::find_if(subsetData.Data.cbegin(), subsetData.Data.cend(),
                 [](PipelineBatchData const& batchData) {
        auto it = std::find_if(batchData.StateDataList.cbegin(), batchData.StateDataList.cend(),
                               [](auto const& stateData) { return stateData.PcaCoordinates.size() > 0; });
        return it != batchData.StateDataList.cend();
    });
    if (it == subsetData.Data.cend())
        throw std::runtime_error("illegal number of pca dimensions");
    uint16_t const numberOfDimensions = it->StateDataList.at(0).PcaCoordinates.size();

    pybind11::object const pcaCoordinateData = interpreter.ExecuteFunction("pca", "calculate",
                                                                           featureData,
                                                                           numberOfDimensions);

    auto const pcaData = pcaCoordinateData.cast<SampleCoordinateData>();

    for (int i = 0, k = 0; i < batchListData.Data.size(); i++) {
        auto& batchData = batchListData.Data.at(i);

        for (int j = 0; j < batchData.StateDataList.size(); j++, k++) {
            auto& stateData = batchData.StateDataList.at(j);

            stateData.PcaCoordinates = pcaData.at(k);
        }
    }

    return batchListData;
}

auto PipelineGroupList::DoTsne(uint8_t numberOfDimensions, ProgressEventCallback const& callback) -> void {
    callback(0.0);

    std::vector<FeatureData const*> featureDataVector;
    for (auto& group: PipelineGroups)
        featureDataVector.emplace_back(&*group->GetFeatureData());

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

auto PipelineGroupList::GetDataStatus() const noexcept -> DataStatus {
    DataStatus status {};

    for (auto const& group: PipelineGroups)
        status.Update(group->GetDataStatus());

    return status;
}

auto PipelineGroupList::GetBatchData() const noexcept -> std::optional<PipelineBatchListData> {
    bool const dataHasBeenGenerated = std::all_of(PipelineGroups.cbegin(), PipelineGroups.cend(),
                                            [](auto const& group) { return group->GetDataStatus().IsComplete(); });
    if (PipelineGroups.empty() || !dataHasBeenGenerated)
        return std::nullopt;

    std::vector<std::vector<std::reference_wrapper<PipelineParameterSpaceState>>> spaceStateVectors;
    std::vector<std::vector<HdfImageReadHandle> const*> imageDataVectors;
    std::vector<FeatureData const*> featureDataVector;
    std::vector<SampleCoordinateData const*> pcaDataVector;
    std::vector<SampleCoordinateData const*> tsneDataVector;

    std::vector<vtkMTimeType> imageMTimes;
    std::vector<vtkMTimeType> featureMTimes;
    std::vector<vtkMTimeType> pcaMTimes;
    std::vector<vtkMTimeType> tsneMTimes;

    for (auto const& group: PipelineGroups) {
        auto const imageData = group->GetImageData();
        auto const featureData = group->GetFeatureData();
        auto const pcaData = group->GetPcaData();
        auto const tsneData = group->GetTsneData();

        spaceStateVectors.emplace_back(group->GetParameterSpaceStates());
        imageDataVectors.emplace_back(&*imageData);
        featureDataVector.emplace_back(&*featureData);
        pcaDataVector.emplace_back(&*pcaData);
        tsneDataVector.emplace_back(&*tsneData);

        imageMTimes.emplace_back(imageData.GetTime());
        featureMTimes.emplace_back(featureData.GetTime());
        pcaMTimes.emplace_back(pcaData.GetTime());
        tsneMTimes.emplace_back(tsneData.GetTime());
    }

    std::vector<std::string> const& featureNames = featureDataVector[0]->Names;

    PipelineBatchListData::StateDataLists stateDataLists;
    stateDataLists.reserve(PipelineGroups.size());

    for (int i = 0; i < imageDataVectors.size(); i++) {
        std::vector<std::reference_wrapper<PipelineParameterSpaceState>> const& spaceStateVector = spaceStateVectors[i];
        std::vector<HdfImageReadHandle> const& imageDataVector = *imageDataVectors[i];
        FeatureData const& featureData = *featureDataVector[i];
        SampleCoordinateData const& pcaData = *pcaDataVector[i];
        SampleCoordinateData const& tsneData = *tsneDataVector[i];

        std::vector<ParameterSpaceStateData> stateDataList;
        stateDataList.reserve(imageDataVectors[0]->size());
        for (int j = 0; j < imageDataVectors[i]->size(); j++)
            stateDataList.emplace_back(spaceStateVector[j], imageDataVector[j],
                                       featureData.Values[j], pcaData[j], tsneData[j]);

        stateDataLists.emplace_back(*PipelineGroups[i], std::move(stateDataList));
    }

    vtkMTimeType const imageMTime   = *std::max_element(imageMTimes.cbegin(), imageMTimes.cend());  // max
    vtkMTimeType const featureMTime = *std::min_element(featureMTimes.cbegin(), featureMTimes.cend());
    vtkMTimeType const pcaMTime     = *std::min_element(pcaMTimes.cbegin(), pcaMTimes.cend());
    vtkMTimeType const tsneMTime    = *std::min_element(tsneMTimes.cbegin(), tsneMTimes.cend());
    vtkMTimeType const totalMTime   = std::min({ imageMTime, featureMTime, pcaMTime, tsneMTime });

    return PipelineBatchListData { *this,
                                   featureNames,
                                   stateDataLists,
                                   { imageMTime, featureMTime, pcaMTime, tsneMTime, totalMTime }};
}

auto PipelineGroupList::ExportGeneratedImages(std::filesystem::path const& exportDir,
                                              PipelineGroupList::ProgressEventCallback const& callback) -> void {
    callback(0.0);

    if (GetDataStatus().Image == 0)
        throw std::runtime_error("Image data has not been generated yet. Cannot export");

    if (!is_directory(exportDir))
        throw std::runtime_error("Given export path is not a directory.");

    auto const exportPath = std::filesystem::path(exportDir) /= ImagesFile.filename();
    std::filesystem::copy(ImagesFile, exportPath, std::filesystem::copy_options::overwrite_existing);

    callback(1.0);
}

auto PipelineGroupList::ImportImages(std::vector<std::filesystem::path> const& importFilePaths,
                                     PipelineGroupList::ProgressEventCallback const& callback) -> void {
    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    callback(0.0);

    for (int i = 0; i < PipelineGroups.size(); i++) {
        std::vector<std::filesystem::path> paths;
        std::copy_if(importFilePaths.cbegin(), importFilePaths.cend(), std::back_inserter(paths),
                     [i](std::filesystem::path const& path) {
            std::regex const regex { std::format(".*(Volume|Mask)-{}-\\d+\\.vtk$", i) };

            return std::regex_match(path.string(), regex);
        });

//        PipelineGroups[i]->ImportImages(paths, ProgressUpdater { i, progressList, callback });
    }
}

auto PipelineGroupList::ExportFeatures(std::filesystem::path const& exportDir,
                                       PipelineGroupList::ProgressEventCallback const& callback) -> void {
    if (!is_directory(exportDir))
        throw std::runtime_error("Given export path is not a directory.");

    if (GetDataStatus().Feature <= 0)
        throw std::runtime_error("cannot export features. they have not been generated yet");

    int const numberOfGroups = PipelineGroups.size();
    int const groupIdx = 1;
    callback(0.1);

    for (auto const& dirEntry : std::filesystem::directory_iterator(PipelineBatch::ExportPathPair::FeatureDirectory)) {
        if (!dirEntry.is_regular_file())
            continue;

        auto const& filePath = dirEntry.path();
        auto const extension = filePath.extension().string();
        if (extension != ".json" && extension != ".csv")
            continue;

        auto const filename = filePath.filename();
        auto const exportPath = std::filesystem::path(exportDir) /= filename;

        std::filesystem::copy(filePath, exportPath, std::filesystem::copy_options::overwrite_existing);

        double const progress = static_cast<double>(groupIdx) / static_cast<double>(numberOfGroups);
        callback(progress);
    }
}

auto PipelineGroupList::ImportFeatures(std::vector<std::filesystem::path> const& importFilePaths,
                                       PipelineGroupList::ProgressEventCallback const& callback) -> void {
    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    callback(0.0);

    for (int i = 0; i < PipelineGroups.size(); i++) {
        auto it = std::find_if(importFilePaths.cbegin(), importFilePaths.cend(),
                     [i](std::filesystem::path const& path) {
                         std::regex const regex { std::format(".*results-{}\\.json$", i) };

                         return std::regex_match(path.string(), regex);
                     });

        if (it == importFilePaths.cend())
            throw std::runtime_error("feature path not found");

        PipelineGroups[i]->ImportFeatures(*it, ProgressUpdater { i, progressList, callback });
    }
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

auto PipelineGroupList::MultiTaskProgressUpdater::operator()(double current) noexcept -> void {
    ProgressList[Idx] = current;

    double const taskFactor = 1.0 / static_cast<double>(NumberOfTasks);
    double const taskOffset = static_cast<double>(CurrentTask) * taskFactor;

    double const taskProgress = (std::reduce(ProgressList.cbegin(), ProgressList.cend())
                                 / static_cast<double>(ProgressList.size()));
    double const totalProgress = taskProgress * taskFactor + taskOffset;

    Callback(totalProgress);
}


std::filesystem::path const PipelineGroupList::DataDirectory = { "..\\data" };
std::filesystem::path const PipelineGroupList::FeatureDirectory
        = (std::filesystem::path(DataDirectory) /= { "features" });
std::filesystem::path PipelineGroupList::ImagesFile {};
