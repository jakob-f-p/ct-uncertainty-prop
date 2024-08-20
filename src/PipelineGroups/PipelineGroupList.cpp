#include "PipelineGroupList.h"

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

#include "nlohmann/json.hpp"

#include "spdlog/spdlog.h"

#include <ranges>
#include <regex>


PipelineGroupList::PipelineGroupList(PipelineList const& pipelines) :
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
    auto const basePipelineTimeIt = std::max_element(basePipelineMTimes.cbegin(), basePipelineMTimes.cend());
    vtkMTimeType const basePipelineTime = basePipelineTimeIt != basePipelineMTimes.cend() ? *basePipelineTimeIt : 0;

    std::vector<vtkMTimeType> pipelineGroupMTimes;
    pipelineGroupMTimes.reserve(PipelineGroups.size());
    for (auto const& group : PipelineGroups)
        pipelineGroupMTimes.emplace_back(group->GetMTime());
    auto const groupMTimeIt = std::max_element(pipelineGroupMTimes.cbegin(), pipelineGroupMTimes.cend());
    vtkMTimeType const groupMTime = groupMTimeIt != pipelineGroupMTimes.cend() ? *groupMTimeIt : 0;

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
    spdlog::debug("Generating images ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    callback(0.0);

    auto const timeStampTime = std::chrono::system_clock::now();
    auto const roundedStartTime = round<std::chrono::seconds>(timeStampTime);
    std::string const timeStampString = std::format("{0:%Y}-{0:%m}-{0:%d}_{0:%H}-{0:%M}-{0:2%S}", timeStampTime);

    ImagesFile = std::filesystem::path(DataDirectory) /= { std::format("images_{}.h5", timeStampString) };

    vtkNew<HdfImageWriter> imageWriter;
    imageWriter->SetFilename(ImagesFile);
    imageWriter->SetArrayNames({ "Radiodensities", "Segmentation Mask" });
    imageWriter->SetTotalNumberOfImages(GetNumberOfPipelines());

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->GenerateImages(*imageWriter, ProgressUpdater { i, progressList, callback });

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    auto const imageDims = App::GetInstance().GetImageDimensions();
    spdlog::info("Generated {} images with dimensions ({}, {}, {}) in {}",
                 GetNumberOfPipelines(),
                 imageDims.at(0), imageDims.at(1), imageDims.at(2),
                 duration);
}

auto PipelineGroupList::ExtractFeatures(PipelineGroupList::ProgressEventCallback const& callback) -> void {
    spdlog::debug("Extracting features ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

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
        PipelineGroups[i]->ExtractFeatures(*imageReader,
                                           WeightedProgressUpdater { i, progressList,
                                                                     groupSizeWeightVector, callback });

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    auto const imageDims = App::GetInstance().GetImageDimensions();
    spdlog::info("Extracted features with dimensions ({}, {}, {}) in {}",
                 imageDims.at(0), imageDims.at(1), imageDims.at(2),
                 duration);
}

auto PipelineGroupList::DoPCAs(uint8_t numberOfDimensions, ProgressEventCallback const& callback) -> void {
    spdlog::debug("Doing PCAs ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < PipelineGroups.size(); i++) {
        double const progress = static_cast<double>(PipelineGroups.size()) / static_cast<double>(i);
        callback(progress);

        PipelineGroups[i]->DoPCA(numberOfDimensions);
    }

    callback(1.0);

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    spdlog::info("Did {} PCAs with {} total samples and {} number of features in {}",
                 PipelineGroups.size(), GetNumberOfPipelines(),
                 PipelineGroups.at(0)->GetFeatureData()->Names.size(),
                 duration);
}

auto PipelineGroupList::DoPCAForSubset(PipelineBatchListData const& subsetData,
                                       ProgressEventCallback const& callback) -> PipelineBatchListData {
    spdlog::debug("Doing PCA for subset ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

    callback(0.1);

    FeatureData featureData { subsetData.FeatureNames, {} };
    for (auto const& batchData : subsetData.Data)
        for (auto const& stateData : batchData.StateDataList)
            featureData.Values.push_back(stateData.FeatureValues);

    auto& interpreter = App::GetInstance().GetPythonInterpreter();
    pybind11::gil_scoped_acquire const acquire {};

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

    auto const pcaData = pcaCoordinateData.cast<PcaData>();

    PipelineBatchListData batchListData { subsetData.GroupList,
                                          subsetData.FeatureNames,
                                          subsetData.Data,
                                          subsetData.Time };
    for (int i = 0, k = 0; i < batchListData.Data.size(); i++) {
        auto& batchData = batchListData.Data.at(i);

        for (int j = 0; j < batchData.StateDataList.size(); j++, k++) {
            auto& stateData = batchData.StateDataList.at(j);

            stateData.PcaCoordinates = pcaData.Values.at(k);
        }

        batchData.PcaExplainedVarianceRatios = pcaData.ExplainedVarianceRatios;
        batchData.PcaPrincipalAxes = pcaData.PrincipalAxes;
    }

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    spdlog::info("Did PCA for subset with {} total samples and {} number of features in {}",
                 featureData.Values.size(), featureData.Names.size(), duration);

    return batchListData;
}

auto PipelineGroupList::DoTsne(uint8_t numberOfDimensions, ProgressEventCallback const& callback) -> void {
    spdlog::debug("Doing t-SNE ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

    callback(0.0);

    std::vector<FeatureData const*> featureDataVector;
    for (auto& group: PipelineGroups)
        featureDataVector.emplace_back(&*group->GetFeatureData());

    auto& interpreter = App::GetInstance().GetPythonInterpreter();

    pybind11::gil_scoped_acquire const acquire {};

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

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    spdlog::info("Did t-SNE with {} total samples and {} number of features in {}",
                 GetNumberOfPipelines(),
                 PipelineGroups.at(0)->GetFeatureData()->Names.size(),
                 duration);
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
    std::vector<PcaData const*> pcaDataVector;
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

    PipelineBatchListData::StateDataLists stateDataLists;
    stateDataLists.reserve(PipelineGroups.size());

    for (int i = 0; i < imageDataVectors.size(); i++) {
        std::vector<std::reference_wrapper<PipelineParameterSpaceState>> const& spaceStateVector
                = spaceStateVectors.at(i);
        std::vector<HdfImageReadHandle> const& imageDataVector = *imageDataVectors.at(i);
        FeatureData const& featureData = *featureDataVector.at(i);
        PcaData const& pcaData = *pcaDataVector.at(i);
        SampleCoordinateData const& tsneData = *tsneDataVector.at(i);

        std::vector<ParameterSpaceStateData> stateDataList;
        stateDataList.reserve(imageDataVectors.at(0)->size());
        for (int j = 0; j < imageDataVectors.at(i)->size(); j++)
            stateDataList.emplace_back(spaceStateVector.at(j), imageDataVector.at(j),
                                       featureData.Values.at(j), pcaData.Values.at(j), tsneData.at(j));

        stateDataLists.emplace_back(*PipelineGroups.at(i),
                                    pcaData.ExplainedVarianceRatios, pcaData.PrincipalAxes,
                                    std::move(stateDataList));
    }

    std::vector<std::string> const& featureNames = featureDataVector.at(0)->Names;

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

auto PipelineGroupList::ExportImagesHdf5(std::filesystem::path const& exportPath,
                                         PipelineGroupList::ProgressEventCallback const& callback) const -> void {
    spdlog::debug("Exporting Images in .hdf5 format ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

    callback(0.0);

    if (GetDataStatus().Image == 0)
        throw std::runtime_error("Image data has not been generated yet. Cannot export");

    if (!is_regular_file(exportPath) && exists(exportPath))
        throw std::runtime_error("Invalid export path");

    std::filesystem::copy(ImagesFile, exportPath, std::filesystem::copy_options::overwrite_existing);

    callback(1.0);

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    spdlog::info("Exported images in .hdf5 format with file size {} MB in {}",
                 file_size(exportPath) / 1000000, duration);
}

auto PipelineGroupList::ExportImagesVtk(std::filesystem::path const& exportDir,
                                        PipelineGroupList::ProgressEventCallback const& callback) -> void {
    spdlog::debug("Exporting Images in .vtk format ...");
    auto const startTime = std::chrono::high_resolution_clock::now();
    auto const startFileTime = std::chrono::file_clock::now();

    callback(0.0);

    if (GetDataStatus().Image == 0)
        throw std::runtime_error("Image data has not been generated yet. Cannot export");

    if (!is_directory(exportDir))
        throw std::runtime_error("Invalid export path");

    std::vector<double> progressList (PipelineGroups.size(), 0.0);
    std::vector<int> groupSizeVector {};
    for (auto const& group : PipelineGroups)
        groupSizeVector.emplace_back(group->GetParameterSpace().GetNumberOfPipelines());
    std::vector<double> groupSizeWeightVector { PipelineGroups.size(), std::allocator<double>{} };
    std::transform(groupSizeVector.cbegin(), groupSizeVector.cend(),
                   groupSizeWeightVector.begin(),
                   [totalSize = GetNumberOfPipelines()](int size) {
        return static_cast<double>(size) / static_cast<double>(totalSize);
    });

    for (int i = 0; i < PipelineGroups.size(); i++)
        PipelineGroups[i]->ExportImagesVtk(exportDir, WeightedProgressUpdater { i, progressList,
                                                                                groupSizeWeightVector, callback });

    callback(1.0);

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);

    std::filesystem::directory_iterator const exportDirIt { exportDir };
    auto&& exportedImages = exportDirIt
            | std::views::filter([startFileTime](std::filesystem::path const& path) {
                return is_regular_file(path) && last_write_time(path) > startFileTime;
            });
    uint32_t const numberOfExportedImages = std::ranges::count_if(exportedImages, [](auto const&) { return true; });
    std::vector<uintmax_t> exportedImagesFileSizes { numberOfExportedImages };
    std::ranges::transform(exportedImages,
                           exportedImagesFileSizes.begin(),
                           [](std::filesystem::path const& file) { return file_size(file); });
    auto const fileSizeSum = std::reduce(exportedImagesFileSizes.begin(), exportedImagesFileSizes.end());
    auto const fileSizeSumMb = fileSizeSum / 1000000;

    spdlog::info("Exported {} images in .vtk format with file size {} MB in {}",
                 GetNumberOfPipelines(), fileSizeSumMb, duration);
}

auto PipelineGroupList::ImportImages(std::filesystem::path const& importFilePath,
                                     PipelineGroupList::ProgressEventCallback const& callback) -> void {
    spdlog::debug("Importing Images ...");
    auto const startTime = std::chrono::high_resolution_clock::now();

    callback(0.0);

    auto const now = round<std::chrono::seconds>(std::chrono::system_clock::now());
    std::string const timeStampString = std::format("{0:%Y}-{0:%m}-{0:%d}_{0:%H}-{0:%M}-{0:2%S}", now);


    for (auto& group : PipelineGroups)
        group->UpdateParameterSpaceStates();

    HdfImageReader::Validate(importFilePath,
                             HdfImageReader::ValidationParameters { GetNumberOfPipelines(),
                                                                    { "Radiodensities", "Segmentation Mask" } });

    bool const isAlreadyInDataDirectory = equivalent(importFilePath.parent_path(), DataDirectory);
    ImagesFile = isAlreadyInDataDirectory
                 ? importFilePath
                 : std::filesystem::path(DataDirectory) /= { std::format("images_{}.h5", timeStampString) };

    if (!isAlreadyInDataDirectory)
        std::filesystem::copy_file(importFilePath, ImagesFile, std::filesystem::copy_options::overwrite_existing);

    for (auto& group : PipelineGroups)
        group->ImportImages();

    callback(1.0);

    auto const endTime = std::chrono::high_resolution_clock::now();
    auto const duration = std::chrono::duration<double>(endTime - startTime);
    spdlog::info("Imported images in .hdf5 format with file size {} MB in {}",
                 file_size(ImagesFile) / 1000000, duration);
}

auto PipelineGroupList::ExportFeatures(std::filesystem::path const& exportPath,
                                       PipelineGroupList::ProgressEventCallback const& callback) -> void {
    if (exists(exportPath) && !is_regular_file(exportPath))
        throw std::runtime_error("Given export path is invalid.");

    if (GetDataStatus().Feature <= 0)
        throw std::runtime_error("Cannot export features. They have not been generated yet");

    size_t const numberOfGroups = PipelineGroups.size();
    size_t const groupIdx = 1;

    callback(0.0);

    using json = nlohmann::json;
    json jsonObject { json::array() };

    for (auto const& group : PipelineGroups) {
        FeatureData features = *group->GetFeatureData();

        json jsonFeatures {};
        for (auto const& values : features.Values) {

            json jsonSampleValues {};

            for (int i = 0; i < values.size(); i++) {
                auto const& name = features.Names.at(i);
                auto const& value = values.at(i);
                jsonSampleValues[name] = value;
            }

            jsonFeatures.emplace_back(std::move(jsonSampleValues));
        }

        jsonObject.emplace_back(std::move(jsonFeatures));
    }

    std::ofstream outStream { exportPath, std::ios::trunc };
    outStream << jsonObject;

    spdlog::info("Exported features");
}

auto PipelineGroupList::ImportFeatures(std::filesystem::path const& importFilePath,
                                       PipelineGroupList::ProgressEventCallback const& callback) -> void {

    if (!is_regular_file(importFilePath))
        throw std::runtime_error("Given import path is invalid.");

    callback(0.0);

    using json = nlohmann::json;
    json jsonObject {};

    std::ifstream inStream { importFilePath };
    inStream >> jsonObject;

    int i = 0;
    for (auto& jsonGroupFeatures : jsonObject) {
        FeatureData featureData {};

        auto& jsonFirstSampleFeatures = jsonGroupFeatures.at(0);
        for (auto const& jsonSampleFeature : jsonFirstSampleFeatures.items()) {
            std::string const& featureName = jsonSampleFeature.key();
            featureData.Names.emplace_back(featureName);
        }

        for (auto& jsonSampleFeatures : jsonGroupFeatures) {
            std::vector<double> featureValues;

            for (const auto& jsonSampleFeature : jsonSampleFeatures.items()) {
                auto const featureValue = jsonSampleFeature.value().get<float>();
                featureValues.emplace_back(featureValue);
            }

            featureData.Values.emplace_back(std::move(featureValues));
        }

        PipelineGroups.at(i)->SetFeatureData(std::move(featureData));

        i++;
    }

    spdlog::info("Imported features");
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
