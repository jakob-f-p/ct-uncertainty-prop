#include "PipelineBatch.h"

#include "ImageScalarsWriter.h"
#include "PipelineGroup.h"
#include "PipelineGroupList.h"
#include "PipelineParameterSpace.h"
#include "PipelineParameterSpaceState.h"
#include "../Utils/PythonInterpreter.h"
#include "../App.h"

#include <pybind11/embed.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkStructuredPointsReader.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <execution>
#include <filesystem>
#include <format>
#include <memory>


PipelineBatch::PipelineBatch(PipelineGroup const& pipelineGroup) :
        Group(pipelineGroup) {

    UpdateParameterSpaceStates();
}

auto PipelineBatch::UpdateParameterSpaceStates() noexcept -> void {
    InitialState = std::make_unique<PipelineParameterSpaceState>(*Group.ParameterSpace);

    PipelineStates pipelineStates;
    pipelineStates.reserve(Group.ParameterSpace->GetNumberOfPipelines());

    auto spaceStates = Group.ParameterSpace->GenerateSpaceStates();

    for (auto& state : spaceStates)
        pipelineStates.emplace_back(std::make_unique<PipelineParameterSpaceState>(std::move(state)));

    States = std::move(pipelineStates);
}

auto PipelineBatch::GenerateImages(ProgressEventCallback const& callback) -> void {
    auto numberOfStates = States.size();

    auto& radiodensitiesAlgorithm = Group.GetBasePipeline().GetImageAlgorithm();
    auto& thresholdAlgorithm = App::GetInstance()->GetThresholdFilter();
    thresholdAlgorithm.SetInputConnection(radiodensitiesAlgorithm.GetOutputPort());

    std::vector<PipelineImageData> batchImageData;
    batchImageData.reserve(numberOfStates);

    for (int i = 0; i < numberOfStates; i++) {
        double const progress = static_cast<double>(i) / static_cast<double>(numberOfStates);
        callback(progress);

        auto& state = States[i];
        if (!state)
            throw std::runtime_error("State must not be nullptr");

        state->Apply();
        thresholdAlgorithm.Update();

        vtkNew<vtkImageData> output;
        output->DeepCopy(thresholdAlgorithm.GetOutput());

        batchImageData.emplace_back(*state, output);
    }

    Images.Emplace(std::move(batchImageData));

    InitialState->Apply();
}

std::filesystem::path const PipelineBatch::ExportPathPair::DataDirectory = { "..\\data" };
std::filesystem::path const PipelineBatch::ExportPathPair::InputDirectory
        = std::filesystem::path(DataDirectory) /= { "input" };
std::filesystem::path const PipelineBatch::ExportPathPair::FeatureDirectory
        = (std::filesystem::path(DataDirectory) /= { "features" });

auto PipelineBatch::ExportImages(ProgressEventCallback const& callback) -> void {
    if (!Images)
        throw std::runtime_error("Image data has not been generated yet. Cannot export");

    std::atomic_uint numberOfExportedImages = 0;
    size_t const numberOfImages = Images->size();
    std::atomic<double> latestProgress = -1.0;

    auto const now = std::chrono::system_clock::now();
    std::string const timeStampString = std::format("{0:%F}ValueT{0:%R%z}", now);

    std::filesystem::create_directory(ExportPathPair::DataDirectory);
    std::filesystem::create_directory(ExportPathPair::InputDirectory);
    std::filesystem::create_directory(ExportPathPair::FeatureDirectory);

    std::vector<ExportPathPair> exportPathPairs { numberOfImages, { {}, {} } };
    ExportedImages.Emplace(std::move(exportPathPairs));

    std::thread imageExportThread ([this, &timeStampString, &latestProgress, &numberOfExportedImages, numberOfImages] {
        std::for_each(std::execution::par_unseq,
                      Images->begin(), Images->end(),
                      DoExport { *this, Group.GroupId, timeStampString,
                                 latestProgress, numberOfExportedImages, numberOfImages });
    });

    while (numberOfExportedImages < numberOfImages) {
        if (latestProgress != -1.0) {
            callback(latestProgress);

            latestProgress = -1.0;
        }
    }

    imageExportThread.join();
}

PYBIND11_EMBEDDED_MODULE(datapaths, m) {
    namespace py = pybind11;

    py::class_<PipelineBatch::ExportPathPair>(m, "PathPair")
            .def(py::init<std::filesystem::path const&, std::filesystem::path const&>())
            .def_readonly("image", &PipelineBatch::ExportPathPair::Radiodensities)
            .def_readonly("mask", &PipelineBatch::ExportPathPair::Mask)
            .def("__repr__", [](PipelineBatch::ExportPathPair const& pathPair) {
                return std::format("({}, {})", pathPair.Radiodensities.string(), pathPair.Mask.string());
            });

    py::class_<PipelineBatch::ExportPathVector>(m, "DataPaths");

    py::class_<FeatureData>(m, "FeatureData")
            .def(py::init<FeatureData::StringVector const&,
                          FeatureData::Vector2DDouble const&>())
            .def_readwrite("names", &FeatureData::Names)
            .def_readwrite("values", &FeatureData::Values)
            .def("__repr__", [](FeatureData const& featureData) {
                std::ostringstream repr;
                repr << "Names: [";
                std::copy(featureData.Names.cbegin(), featureData.Names.cend(),
                          std::ostream_iterator<std::string>(repr, ", "));
                repr << "]\nData: [";
                for (auto const& row : featureData.Values) {
                    repr << "[";
                    std::transform(row.cbegin(), row.cend(),
                                   std::ostream_iterator<std::string>(repr, ", "),
                                   [](double value) { return std::to_string(value); });
                    repr << "]\n";
                }
                repr << "]\n";
                return repr.str();
            });

    m.attr("extraction_params_file") = std::filesystem::path { FEATURE_EXTRACTION_PARAMETERS_FILE }.make_preferred();
    m.attr("feature_directory") = PipelineBatch::ExportPathPair::FeatureDirectory;
}

auto PipelineBatch::ExtractFeatures(ProgressEventCallback const& callback) -> void {
    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    pybind11::object const featureObject = interpreter.ExecuteFunction("extract_features", "extract",
                                                                       Group.GroupId, *ExportedImages, callback);

    Features.Emplace(featureObject.cast<FeatureData>());
}

auto PipelineBatch::DoPCA(uint8_t numberOfDimensions) -> void {
    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    pybind11::object const pcaCoordinateData = interpreter.ExecuteFunction("pca", "calculate",
                                                                           *Features, numberOfDimensions);

    PcaData.Emplace(pcaCoordinateData.cast<SampleCoordinateData>());
}

auto PipelineBatch::GetImageData() -> TimeStampedData<std::vector<PipelineImageData*>> {
    std::vector<PipelineImageData*> imageDataVector;
    imageDataVector.reserve(Images->size());

    for (auto& imageData : *Images)
        imageDataVector.emplace_back(&imageData);

    return TimeStampedData<std::vector<PipelineImageData*>> { std::move(imageDataVector), Images.GetTime() };
}

auto PipelineBatch::GetFeatureData() const -> TimeStampedDataRef<FeatureData> {
    return TimeStampedDataRef<FeatureData> { Features };
}

auto PipelineBatch::GetPcaData() const -> TimeStampedDataRef<SampleCoordinateData> {
    return TimeStampedDataRef<SampleCoordinateData> { PcaData };
}

auto PipelineBatch::GetTsneData() const -> TimeStampedDataRef<SampleCoordinateData> {
    return TimeStampedDataRef<SampleCoordinateData> { TsneData };
}

auto PipelineBatch::SetTsneData(SampleCoordinateData&& tsneData) -> void {
    TsneData.Emplace(std::move(tsneData));
}

auto PipelineBatch::GetDataStatus() const noexcept -> DataStatus {
    return { Images.GetTime(), Features.GetTime(), PcaData.GetTime(), TsneData.GetTime() };
}

auto PipelineBatch::ExportGeneratedImages(std::filesystem::path const& exportDir,
                                          PipelineBatch::ProgressEventCallback const& callback) -> void {
    if (!ExportedImages)
        throw std::runtime_error("Image data has not been generated yet. Cannot export");

    if (!is_directory(exportDir))
        throw std::runtime_error("Given export path is not a directory.");

    std::atomic_uint numberOfExportedImages = 0;
    size_t const numberOfImages = ExportedImages->size();
    std::atomic<double> latestProgress = -1.0;

    std::thread imageExportThread ([this, &exportDir, &numberOfExportedImages, numberOfImages, &latestProgress] {
        std::for_each(std::execution::par_unseq,
                      ExportedImages->begin(), ExportedImages->end(),
                      [&exportDir, &numberOfExportedImages, &latestProgress, numberOfImages]
                      (ExportPathPair const& pathPair) {
            auto const volumeInMemoryPath = pathPair.Radiodensities;
            auto const volumeExportPath = std::filesystem::path(exportDir) /= volumeInMemoryPath.filename();

            auto const maskInMemoryPath = pathPair.Mask;
            auto const maskExportPath = std::filesystem::path(exportDir) /= maskInMemoryPath.filename();

            std::filesystem::copy(volumeInMemoryPath, volumeExportPath, std::filesystem::copy_options::overwrite_existing);
            std::filesystem::copy(maskInMemoryPath, maskExportPath, std::filesystem::copy_options::overwrite_existing);

            latestProgress = static_cast<double>(numberOfExportedImages++ + 1) / static_cast<double>(numberOfImages);
        });
    });

    while (numberOfExportedImages < numberOfImages) {
        if (latestProgress != -1.0) {
            callback(latestProgress);

            latestProgress = -1.0;
        }
    }

    imageExportThread.join();
}

auto PipelineBatch::ImportImages(std::vector<std::filesystem::path> const& importFilePaths,
                                 PipelineBatch::ProgressEventCallback const& callback) -> void {
    if (!std::all_of(importFilePaths.cbegin(), importFilePaths.cend(),
                     [](std::filesystem::path const& path) { return is_regular_file(path); }))
        throw std::runtime_error("not all given paths are files");

    std::vector<std::filesystem::path> volumePaths;
    std::copy_if(importFilePaths.cbegin(), importFilePaths.cend(), std::back_inserter(volumePaths),
                 [](std::filesystem::path const& path) { return path.string().find("Volume") != std::string::npos; });
    std::vector<std::filesystem::path> maskPaths;
    std::copy_if(importFilePaths.cbegin(), importFilePaths.cend(), std::back_inserter(maskPaths),
                 [](std::filesystem::path const& path) { return path.string().find("Mask") != std::string::npos; });

    UpdateParameterSpaceStates();
    if (volumePaths.size() != States.size() || maskPaths.size() != States.size())
        throw std::runtime_error("invalid number of import paths");

    std::vector<ExportPathPair> importPaths;
    for (int i = 0; i < States.size(); i++)
        importPaths.emplace_back(volumePaths[i], maskPaths[i]);

    std::atomic_uint numberOfImportedImages = 0;
    size_t const numberOfImages = States.size();
    std::atomic<double> latestProgress = -1.0;

    std::thread imageImportThread ([&importPaths, &numberOfImportedImages, numberOfImages, &latestProgress] {
        std::for_each(std::execution::par_unseq,
                      importPaths.begin(), importPaths.end(),
                      [&numberOfImportedImages, &latestProgress, numberOfImages](ExportPathPair const& pathPair) {
           auto const volumeImportPath = pathPair.Radiodensities;
           auto const volumeInMemoryPath
                 = std::filesystem::path(ExportPathPair::InputDirectory) /= volumeImportPath.filename();

           auto const maskImportPath = pathPair.Mask;
           auto const maskInMemoryPath
                 = std::filesystem::path(ExportPathPair::InputDirectory) /= maskImportPath.filename();

           std::filesystem::copy(volumeImportPath, volumeInMemoryPath, std::filesystem::copy_options::overwrite_existing);
           std::filesystem::copy(maskImportPath, maskInMemoryPath, std::filesystem::copy_options::overwrite_existing);

           vtkNew<vtkStructuredPointsReader> reader;

           reader->SetFileName(volumeInMemoryPath.string().c_str());
           reader->Update();
           reader->GetOutput();

           latestProgress = static_cast<double>(numberOfImportedImages++ + 1) / static_cast<double>(numberOfImages);
      });
    });

    while (numberOfImportedImages < numberOfImages) {
        if (latestProgress != -1.0) {
            callback(latestProgress);

            latestProgress = -1.0;
        }
    }

    imageImportThread.join();
}

auto PipelineBatch::ImportFeatures(std::filesystem::path const& importFilePath,
                                   PipelineBatch::ProgressEventCallback const& callback) -> void {
    if (!is_regular_file(importFilePath))
        throw std::runtime_error("given path is not a file");

    UpdateParameterSpaceStates();

    callback(0.0);

    auto const inMemoryPath = std::filesystem::path(ExportPathPair::FeatureDirectory) /= importFilePath.filename();

    std::filesystem::copy(importFilePath, inMemoryPath, std::filesystem::copy_options::overwrite_existing);

    using json = nlohmann::json;
    std::ifstream ifs { inMemoryPath };
    json const data = json::parse(ifs);

    FeatureData featureData;
    for (auto const& resultEntry : data.at(0).items()) {
        if (resultEntry.key().starts_with("original_"))
            featureData.Names.emplace_back(resultEntry.key());
    }

    int pipelineIdx = 0;
    for (auto const& pipelineResult : data) {
        std::vector<double> pipelineFeatures {};
        pipelineFeatures.reserve(featureData.Names.size());

        for (auto const& featureName : featureData.Names) {
            double const value = pipelineResult[featureName];
            pipelineFeatures.emplace_back(value);
        }

        featureData.Values.emplace_back(std::move(pipelineFeatures));

        pipelineIdx++;
    }

    if (featureData.Values.size() != States.size())
        throw std::runtime_error("invalid number of pipelines in feature result file");

    Features.Emplace(featureData);

    callback(1.0);
}

auto PipelineBatch::GetIdx(PipelineParameterSpaceState const& state) const -> uint32_t {
    auto it = std::find_if(States.cbegin(), States.cend(),
                           [&state](auto const& s) { return s.get() == &state; });

    if (it == States.cend())
        throw std::runtime_error("State not found");

    return std::distance(States.cbegin(), it);
}

auto PipelineBatch::DoExport::operator()(PipelineImageData& image) const -> void {
    using std::filesystem::path;

    uint32_t const stateIdx = Batch.GetIdx(image.State);
    path const imagePath
            = path(ExportPathPair::InputDirectory) /= path(std::format("Volume-{}-{}.vtk", GroupIdx, stateIdx));
    path const maskPath
            = path(ExportPathPair::InputDirectory) /= path(std::format("Mask-{}-{}.vtk", GroupIdx, stateIdx));
//    path const segmentationPath
//            = path(ExportPathPair::InputDirectory) /= path(std::format("Segmentation-{}-{}.vtk", GroupIdx, stateIdx));

    vtkNew<ImageScalarsWriter> imageExporter;
    imageExporter->SetInputData(image.ImageData);
    imageExporter->SetHeader(TimeStampString.c_str());
    imageExporter->WriteExtentOff();

    imageExporter->SetFileName(maskPath.string().c_str());
    imageExporter->SetScalarsArrayName("Segmentation Mask");
    imageExporter->Write();

    imageExporter->SetFileName(imagePath.string().c_str());
    imageExporter->SetScalarsArrayName("Radiodensities");
    imageExporter->Write();

//    imageExporter->SetFileName(segmentationPath.string().c_str());
//    imageExporter->SetScalarsArrayName("Segmented Radiodensities");
//    imageExporter->Write();

    (*Batch.ExportedImages)[stateIdx] = { imagePath, maskPath };

    Progress = static_cast<double>(NumberOfExportedImages++ + 1) / static_cast<double>(NumberOfImages);
}
