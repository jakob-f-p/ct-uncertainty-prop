#include "PipelineGroup.h"

#include "PipelineGroupList.h"
#include "PipelineParameterSpace.h"
#include "PipelineParameterSpaceState.h"

#include "IO/ImageScalarsWriter.h"
#include "IO/HdfImageReader.h"
#include "IO/HdfImageWriter.h"
#include "../Artifacts/Pipeline.h"
#include "../Modeling/CtDataSource.h"
#include "../Modeling/CtStructureTree.h"
#include "../App.h"
#include "../Utils/PythonInterpreter.h"
#include "../Utils/System.h"

#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkTypeInt16Array.h>

#include <nlohmann/json.hpp>

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <chrono>
#include <memory>


PipelineGroup::PipelineGroup(Pipeline const& basePipeline, std::string name) :
        Name(name.empty()
             ? "Pipeline Group " + std::to_string(PipelineGroupId++)
             : std::move(name)),
        GroupId(PipelineGroupId++),
        BasePipeline(basePipeline),
        ParameterSpace(new PipelineParameterSpace()),
        Data({}) {}

PipelineGroup::~PipelineGroup() = default;

auto PipelineGroup::GetName() const noexcept -> std::string {
    return Name;
}

auto PipelineGroup::GetMTime() const noexcept -> vtkMTimeType {
    return ParameterSpace->GetMTime();
}

auto PipelineGroup::GetBasePipeline() const noexcept -> Pipeline const& {
    return BasePipeline;
}

auto PipelineGroup::GetParameterSpace() noexcept -> PipelineParameterSpace& {
    return *ParameterSpace;
}

auto PipelineGroup::GetParameterSpace() const noexcept -> PipelineParameterSpace const& {
    return *ParameterSpace;
}

auto PipelineGroup::GenerateImages(HdfImageWriter& imageWriter, ProgressEventCallback const& callback) -> void {
    UpdateParameterSpaceStates();

    auto const numberOfStates = Data.States.size();

    auto& radiodensitiesAlgorithm = GetBasePipeline().GetImageAlgorithm();
    auto& thresholdAlgorithm = App::GetInstance()->GetThresholdFilter();
    thresholdAlgorithm.SetInputConnection(radiodensitiesAlgorithm.GetOutputPort());

    HdfImageReadHandles imageReadHandles;
    imageReadHandles.reserve(numberOfStates);

    uint64_t const maxBatchSize = GetMaxImageBatchSize();

    for (int i = 0; i < numberOfStates;) {
        uint64_t const currentBatchSize = i + maxBatchSize < numberOfStates
                ? maxBatchSize
                : std::min({ maxBatchSize, numberOfStates - i });

        std::vector<vtkNew<vtkImageData>> batchImageData { currentBatchSize };
        HdfImageWriter::BatchImages batchImages {};
        auto const generateStartTime = std::chrono::system_clock::now();
        for (auto& imageData : batchImageData) {
            double const progress = static_cast<double>(i) / static_cast<double>(numberOfStates);
            callback(progress);

            auto& state = Data.States[i];
            if (!state)
                throw std::runtime_error("State must not be nullptr");

            state->Apply();
            thresholdAlgorithm.Update();
            imageData->ShallowCopy(thresholdAlgorithm.GetOutput());
            thresholdAlgorithm.SetOutput(vtkNew<vtkImageData>());

            SampleId const sampleId { GroupId, static_cast<uint16_t>(i) };
            batchImages.push_back({ sampleId, *imageData });
            imageReadHandles.push_back({ PipelineGroupList::ImagesFile, sampleId });

            i++;
        }

        auto const generateEndTime = std::chrono::system_clock::now();
        auto const generateDuration = std::chrono::duration_cast<std::chrono::seconds>(generateEndTime - generateStartTime);
        std::cout << std::format("generated ({}), {}", i, generateDuration) << std::endl;

        std::cout << "before write" << std::endl;
        auto const writeStartTime = std::chrono::system_clock::now();

        imageWriter.SetBatch(std::move(batchImages));
        imageWriter.Write();

        auto const writeEndTime = std::chrono::system_clock::now();
        auto const writeDuration = std::chrono::duration_cast<std::chrono::seconds>(writeEndTime - writeStartTime);
        std::cout << std::format("after write: {}", writeDuration) << std::endl;
    }

    Data.InitialState->Apply();

    Data.Images.Emplace(std::move(imageReadHandles));
}

PYBIND11_EMBEDDED_MODULE(feature_extraction_cpp, m) {
    namespace py = pybind11;

    enum struct VtkType : uint8_t { SHORT = VTK_SHORT, FLOAT = VTK_FLOAT };

//    py::enum_<VtkType>(m, "VtkType")
//            .value("Short", VtkType::SHORT)
//            .value("Float", VtkType::FLOAT)
//            .export_values();

    m.attr("VtkType_Short") = static_cast<uint8_t>(VtkType::SHORT);
    m.attr("VtkType_Float") = static_cast<uint8_t>(VtkType::FLOAT);

    py::class_<vtkImageData, std::unique_ptr<vtkImageData, py::nodelete>>(m, "VtkImageData", py::buffer_protocol())
            .def("get_dimensions", [](vtkImageData& imageData) {
                int* dims = imageData.GetDimensions();
                return std::vector<int> { dims, std::next(dims, 3) };
            })
            .def("get_origin", [](vtkImageData& imageData) {
                double* origin = imageData.GetOrigin();
                return std::vector<double> { origin, std::next(origin, 3) };
            })
            .def("get_spacing", [](vtkImageData& imageData) {
                double* spacing = imageData.GetSpacing();
                return std::vector<double> { spacing, std::next(spacing, 3) };
            })
            .def("get_data_type", [](vtkImageData& imageData) {
                int const vtkType = imageData.GetPointData()->GetScalars()->GetDataType();
                return static_cast<uint8_t>(vtkType);
            })
            .def_buffer([](vtkImageData& imageData) -> py::buffer_info {
                auto* scalarAbstractArray = imageData.GetPointData()->GetScalars();
                int const vtkDataType = scalarAbstractArray->GetDataType();
                std::variant<float, short> vtkDataTypeHolder = [vtkDataType]() -> std::variant<float, short> {
                    switch (vtkDataType) {
                        case VTK_FLOAT: return static_cast<float>(0);
                        case VTK_SHORT: return static_cast<short>(0);
                        default: throw std::runtime_error("vtk data type not supported");
                    }
                }();

                return std::visit([scalarAbstractArray](auto dataTypeHolder) {
                    using ValueType = decltype(dataTypeHolder);
                    using VtkArrayType = std::conditional_t<std::is_same_v<ValueType, float>,
                                                            vtkFloatArray,
                                                            std::conditional_t<std::is_same_v<ValueType, short>,
                                                                               vtkTypeInt16Array,
                                                                               nullptr_t>>;
                    auto* scalarArray = VtkArrayType::SafeDownCast(scalarAbstractArray);
                    return py::buffer_info { scalarArray->GetPointer(0), scalarArray->GetNumberOfTuples(), true };
                }, vtkDataTypeHolder);
            });

    using ImageMaskRefPair = PipelineGroup::ImageMaskRefPair;

    py::class_<ImageMaskRefPair>(m, "VtkImageMaskPair")
            .def_readonly("image", &ImageMaskRefPair::Image)
            .def_readonly("mask", &ImageMaskRefPair::Mask);

    py::class_<FeatureData>(m, "FeatureData")
            .def(py::init<FeatureData::StringVector const&, FeatureData::Vector2DDouble const&>())
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

    py::class_<SampleId>(m, "SampleId")
            .def(py::init<uint16_t, uint16_t>())
            .def_readwrite("group_idx", &SampleId::GroupIdx)
            .def_readwrite("state_idx", &SampleId::StateIdx)
            .def("__repr__", [](SampleId const& id) { return std::format("({}, {})", id.GroupIdx, id.StateIdx); });

    m.attr("extraction_params_file") = std::filesystem::path { FEATURE_EXTRACTION_PARAMETERS_FILE }.make_preferred();
    m.attr("feature_directory") = PipelineGroupList::FeatureDirectory;
}

auto PipelineGroup::ExtractFeatures(HdfImageReader& imageReader, ProgressEventCallback const& callback) -> void {
    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    pybind11::gil_scoped_acquire const acquire {};

    uint64_t const numberOfStates = Data.States.size();
    uint64_t const maxBatchSize = GetMaxImageBatchSize();

    uint64_t const numberOfTasks = 3;

    callback(0.0);

    std::vector<FeatureData> batchFeatureDataVector {};
    for (uint16_t i = 0; i < numberOfStates;) {
        uint64_t const currentBatchSize = i + maxBatchSize < numberOfStates
                ? maxBatchSize
                : std::min({ maxBatchSize, numberOfStates - i });

        double const baseProgress = static_cast<double>(i) / static_cast<double>(numberOfStates);
        double const progressPerTask = static_cast<double>(currentBatchSize)
                                               / static_cast<double>(numberOfStates)
                                               / static_cast<double>(numberOfTasks);
        callback(baseProgress);

        std::vector<vtkNew<vtkImageData>> batchImageData { currentBatchSize };
        HdfImageReader::BatchImages batchImages {};
        for (int k = 0; k < currentBatchSize; k++) {
            auto& imageData = *batchImageData.at(k);
            batchImages.emplace_back(SampleId { GroupId, static_cast<uint16_t>(i + k) }, imageData);
        }

        imageReader.ReadImageBatch(batchImages);

        callback(baseProgress + progressPerTask);

        struct ImageMaskPair {
            vtkNew<vtkImageData> Image;
            vtkNew<vtkImageData> Mask;
        };

        std::vector<ImageMaskPair> batchImageMaskPairs { currentBatchSize };
        std::vector<ImageMaskRefPair> batchImageMaskRefPairs {};
        for (int k = 0; k < currentBatchSize; k++) {
            auto& imageData = *batchImageData.at(k);
            auto& pair = batchImageMaskPairs.at(k);

            auto& image = *pair.Image;
            image.ShallowCopy(&imageData);
            image.GetPointData()->SetActiveScalars("Radiodensities");

            auto& mask = *pair.Mask;
            mask.ShallowCopy(&imageData);
            mask.GetPointData()->SetActiveScalars("Segmentation Mask");

            batchImageMaskRefPairs.emplace_back(image, mask);
        }

        pybind11::object const featureObject = interpreter.ExecuteFunction("extract_features",
                                                                           "extract",
                                                                           batchImageMaskRefPairs);

        batchFeatureDataVector.emplace_back(featureObject.cast<FeatureData>());

        callback(baseProgress + 2.0 * progressPerTask);

        i += currentBatchSize;
    }

    FeatureData featureData { batchFeatureDataVector.at(0).Names, {} };
    for (auto const& batchFeatureData : batchFeatureDataVector)
        featureData.Values.insert(featureData.Values.cend(),
                                  batchFeatureData.Values.begin(),
                                  batchFeatureData.Values.end());

    Data.Features.Emplace(std::move(featureData));
}

auto PipelineGroup::DoPCA(uint8_t numberOfDimensions) -> void {
    auto& interpreter = App::GetInstance()->GetPythonInterpreter();

    pybind11::gil_scoped_acquire const acquire {};

    pybind11::object const pcaCoordinateData = interpreter.ExecuteFunction("pca", "calculate",
                                                                           *Data.Features, numberOfDimensions);

    Data.PcaData.Emplace(pcaCoordinateData.cast<SampleCoordinateData>());
}

auto PipelineGroup::GetParameterSpaceStates() -> std::vector<std::reference_wrapper<PipelineParameterSpaceState>> {
    std::vector<std::reference_wrapper<PipelineParameterSpaceState>> states;

    std::transform(Data.States.begin(), Data.States.end(),
                   std::back_inserter(states),
                   [](auto& stateUniquePointer) -> PipelineParameterSpaceState& {
        return *stateUniquePointer.get();
    });

    return states;
}

auto PipelineGroup::GetImageData() const -> TimeStampedDataRef<HdfImageReadHandles> {
    return TimeStampedDataRef<HdfImageReadHandles> { Data.Images };
}

auto PipelineGroup::GetFeatureData() const -> TimeStampedDataRef<FeatureData> {
    return TimeStampedDataRef<FeatureData> { Data.Features };
}

auto PipelineGroup::GetPcaData() const -> TimeStampedDataRef<SampleCoordinateData> {
    return TimeStampedDataRef<SampleCoordinateData> { Data.PcaData };
}

auto PipelineGroup::GetTsneData() const -> TimeStampedDataRef<SampleCoordinateData> {
    return TimeStampedDataRef<SampleCoordinateData> { Data.TsneData };
}

auto PipelineGroup::SetFeatureData(FeatureData&& featureData) -> void {
    Data.Features.Emplace(std::move(featureData));
}

auto PipelineGroup::SetTsneData(SampleCoordinateData&& tsneData) -> void {
    Data.TsneData.Emplace(std::move(tsneData));
}

auto PipelineGroup::GetDataStatus() const noexcept -> DataStatus {
    return { Data.Images.GetTime(), Data.Features.GetTime(), Data.PcaData.GetTime(), Data.TsneData.GetTime() };
}

auto PipelineGroup::ImportImages() -> void {
    UpdateParameterSpaceStates();

    auto const numberOfStates = Data.States.size();

    HdfImageReadHandles imageReadHandles;
    imageReadHandles.reserve(numberOfStates);

    for (int i = 0; i < numberOfStates; i++)
        imageReadHandles.push_back({ PipelineGroupList::ImagesFile,
                                     SampleId { GroupId, static_cast<uint16_t>(i) } });

    Data.Images.Emplace(std::move(imageReadHandles));
}

auto PipelineGroup::ExportImagesVtk(std::filesystem::path const& exportDir,
                                    PipelineGroup::ProgressEventCallback const& callback) -> void {
    std::atomic_uint const numberOfExportedImages = 0;
    size_t const numberOfImages = Data.Images->size();

    auto const now = std::chrono::system_clock::now();
    std::string const timeStampString = std::format("{0:%F}ValueT{0:%R%z}", now);

    for (int i = 0; i < numberOfImages; i++) {
        double const progress = static_cast<double>(i) / static_cast<double>(numberOfImages);
        callback(progress);

        auto& imageReadHandle = Data.Images->at(i);
        auto image = imageReadHandle.Read({ "Radiodensities", "Segmentation Mask" });
        using std::filesystem::path;

        uint32_t const stateIdx = i;
        path const imagePath = path(exportDir) /= path(std::format("Volume-{}-{}.vtk", GroupId, stateIdx));
        path const maskPath =  path(exportDir) /= path(std::format("Mask-{}-{}.vtk",   GroupId, stateIdx));

        vtkNew<ImageScalarsWriter> imageExporter;
        imageExporter->SetInputData(image);
        imageExporter->SetHeader(timeStampString.c_str());
        imageExporter->WriteExtentOff();

        imageExporter->SetFileName(imagePath.string().c_str());
        imageExporter->SetScalarsArrayName("Radiodensities");
        imageExporter->Write();

        imageExporter->SetFileName(maskPath.string().c_str());
        imageExporter->SetScalarsArrayName("Segmentation Mask");
        imageExporter->Write();
    }
}

auto PipelineGroup::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                     PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan& {
    return ParameterSpace->AddParameterSpan(artifactVariantPointer, std::move(parameterSpan));
}

auto PipelineGroup::RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                        PipelineParameterSpan const& parameterSpan) -> void {
    ParameterSpace->RemoveParameterSpan(artifactVariantPointer, parameterSpan);
}

uint16_t PipelineGroup::PipelineGroupId = 0;

auto PipelineGroup::UpdateParameterSpaceStates() noexcept -> void {
    Data.InitialState = std::make_unique<PipelineParameterSpaceState>(*ParameterSpace);

    SpaceStates spaceStateUniquePointers;
    spaceStateUniquePointers.reserve(ParameterSpace->GetNumberOfPipelines());

    auto spaceStates = ParameterSpace->GenerateSpaceStates();

    for (auto& state : spaceStates)
        spaceStateUniquePointers.emplace_back(std::make_unique<PipelineParameterSpaceState>(std::move(state)));

    Data.States = std::move(spaceStateUniquePointers);
}

auto PipelineGroup::GetMaxImageBatchSize() -> uint64_t {
    auto const& dataSource = App::GetInstance()->GetCtDataSource();
    auto const dimensions = dataSource.GetVolumeNumberOfVoxels();
    uint64_t const numberOfVoxels = std::reduce(dimensions.cbegin(), dimensions.cend(), 1, std::multiplies {});

    auto const imageSizeInBytes = numberOfVoxels * sizeof(float) * 10;
    auto const applicationMemoryMaxSize = System::GetMaxApplicationMemory();

    uint64_t const maxNumberOfImages = applicationMemoryMaxSize / imageSizeInBytes;

    std::cout << std::format("imageSize (B): {}\napplicationMemoryMaxSize: {}\nmaxNumberImages: {}",
                             imageSizeInBytes, applicationMemoryMaxSize, maxNumberOfImages) << std::endl;

    return 1;
    return std::max({ maxNumberOfImages, 1ULL });
}
