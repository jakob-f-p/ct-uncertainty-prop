#include "PipelineBatch.h"

#include "ImageScalarsWriter.h"
#include "PipelineGroup.h"
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

auto PipelineBatch::GetIdx(PipelineParameterSpaceState const& state) const -> uint32_t {
    auto it = std::find_if(States.cbegin(), States.cend(),
                           [&state](auto const& s) { return s.get() == &state; });

    if (it == States.cend())
        throw std::runtime_error("State not found");

    return std::distance(States.cbegin(), it);
}
