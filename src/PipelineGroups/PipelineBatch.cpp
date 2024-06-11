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
        }()),
        Images(States.size(), std::nullopt) {}

auto PipelineBatch::GenerateImages(ProgressEventCallback const& callback) -> void {
    auto numberOfStates = States.size();

    for (int i = 0; i < numberOfStates; i++) {
        double const progress = static_cast<double>(i) / static_cast<double>(numberOfStates);
        callback(progress);

        auto& state = States[i];
        if (!state)
            throw std::runtime_error("State must not be nullptr");

        auto& radiodensitiesAlgorithm = Group.GetBasePipeline().GetImageAlgorithm();
        auto& thresholdAlgorithm = App::GetInstance()->GetThresholdFilter();
        thresholdAlgorithm.SetInputConnection(radiodensitiesAlgorithm.GetOutputPort());

        state->Apply();
        thresholdAlgorithm.Update();

        Images[i].emplace(*state, thresholdAlgorithm.GetOutput(), std::nullopt);
    }

    InitialState->Apply();
}

std::filesystem::path const PipelineBatch::ExportPathPair::DataDirectory = { "..\\data" };
std::filesystem::path const PipelineBatch::ExportPathPair::InputDirectory
        = std::filesystem::path(DataDirectory) /= { "input" };
std::filesystem::path const PipelineBatch::ExportPathPair::FeatureDirectory
        = (std::filesystem::path(DataDirectory) /= { "features" });

auto PipelineBatch::ExportImages(uint32_t groupIdx, ProgressEventCallback const& callback) -> void {
    std::atomic_uint numberOfExportedImages = 0;
    size_t const numberOfImages = Images.size();
    std::atomic<double> latestProgress = -1.0;

    auto const now = std::chrono::system_clock::now();
    std::string const timeStampString = std::format("{0:%F}T{0:%R%z}", now);

    std::filesystem::create_directory(ExportPathPair::DataDirectory);
    std::filesystem::create_directory(ExportPathPair::InputDirectory);
    std::filesystem::create_directory(ExportPathPair::FeatureDirectory);

    std::thread imageExportThread ([this, groupIdx, &timeStampString,
                                    &latestProgress, &numberOfExportedImages, numberOfImages]() {
        std::for_each(std::execution::par_unseq,
                      Images.begin(), Images.end(),
                      DoExport { *this, groupIdx, timeStampString,
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

auto PipelineBatch::GetIdx(PipelineParameterSpaceState const& state) const -> uint32_t {
    auto it = std::find_if(States.cbegin(), States.cend(),
                           [&state](auto const& s) { return s.get() == &state; });

    if (it == States.cend())
        throw std::runtime_error("State not found");

    return std::distance(States.cbegin(), it);
}

PYBIND11_EMBEDDED_MODULE(datapaths, m) {
    namespace py = pybind11;

    py::class_<PipelineBatch::ExportPathPair>(m, "PathPair")
            .def(py::init<std::filesystem::path const&, std::filesystem::path const&>())
            .def_readonly("image", &PipelineBatch::ExportPathPair::Radiodensities)
            .def_readonly("mask", &PipelineBatch::ExportPathPair::Mask);

    py::class_<PipelineBatch::ExportPathVector>(m, "DataPaths");
}

auto PipelineBatch::ExtractFeatures(ProgressEventCallback const& callback) -> void {
    ExportPathVector exportPathVector;
    exportPathVector.reserve(Images.size());
    std::transform(Images.cbegin(), Images.cend(), std::back_inserter(exportPathVector), [](auto const& optionalImage) {
        if (!optionalImage || !(*optionalImage).Paths)
            throw std::runtime_error("Not all image data has been generated and exported");

        return *((*optionalImage).Paths);
    });

    std::filesystem::path const featureExtractionParametersFile
            = std::filesystem::path { FEATURE_EXTRACTION_PARAMETERS_FILE }.make_preferred();

    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    interpreter.ExecuteFunction("extract_features", "extract",
                                featureExtractionParametersFile,
                                exportPathVector,
                                ExportPathPair::FeatureDirectory,
                                callback);
}

auto PipelineBatch::DoExport::operator()(std::optional<Image>& optionalImage) const -> void {
    if (!optionalImage)
        throw std::runtime_error("Image data has not been generated yet. Cannot export");

    auto& image = *optionalImage;

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
//    imageExporter->WriteExtentOn();
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

    image.Paths.emplace(imagePath, maskPath);

    Progress = static_cast<double>(NumberOfExportedImages++ + 1) / static_cast<double>(NumberOfImages);
}
