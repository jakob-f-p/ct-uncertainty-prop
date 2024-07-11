#include "HdfImageWriter.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkTypeInt16Array.h>

#include <highfive/highfive.hpp>

#include <ranges>
#include <variant>


vtkStandardNewMacro(HdfImageWriter);

auto HdfImageWriter::SetBatch(HdfImageWriter::BatchImages&& images) noexcept -> void {
    if (Batch == images)
        return;

    Batch = std::move(images);

    MaxSampleId = std::ranges::max_element(Batch, {}, &BatchImage::Id)->Id;

    RemoveAllInputs();
    std::ranges::for_each(Batch,
                          [this](BatchImage& batchImage) { AddInputDataObject(&batchImage.ImageData); });

    Modified();
}

void HdfImageWriter::WriteData() {
    int const numberOfInputConnections = GetNumberOfInputConnections(0);

    if (Filename.empty())
        throw std::runtime_error("filename must not be empty");

    if (ArrayNames.empty())
        throw std::runtime_error("array names must not be empty");

    InputImages.clear();
    for (int i = 0; i < numberOfInputConnections; i++)
        InputImages.emplace_back(*vtkImageData::SafeDownCast(GetInputDataObject(0, i)));

    unsigned int const openFlags = TruncateFileBeforeWrite ? HighFive::File::Truncate : HighFive::File::ReadWrite;
    auto file = HighFive::File(Filename.string(), openFlags);

    if (InputImages.empty())
        throw std::runtime_error("input images must not be empty");

    if (TruncateFileBeforeWrite)
        InitializeFile(file);

    WriteImageBatch(file);
}

auto HdfImageWriter::FillInputPortInformation(int port, vtkInformation* info) -> int {
    if (port == 0) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
        info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);

        return 1;
    }

    return 0;
}

auto HdfImageWriter::InitializeFile(HighFive::File& file) -> void {
    using HighFive::AtomicType;
    using HighFive::CompoundType;

    auto& image = InputImages.at(0).get();

    std::vector<int> const imageDimensions { image.GetDimensions(), std::next(image.GetDimensions(), 3) };
    size_t const numberOfElements = std::reduce(imageDimensions.cbegin(), imageDimensions.cend(), 1, std::multiplies{});
    HighFive::DataSpace const dataSpace { TotalNumberOfImages, numberOfElements };

    for (auto const& arrayName : ArrayNames) {
        auto* pointData = image.GetPointData();
        if (!pointData)
            throw std::runtime_error("point data must not be null");

        auto* abstractArray = pointData->GetAbstractArray(arrayName.data());
        if (!abstractArray)
            throw std::runtime_error("abstract array must not be null");

        int const vtkDataType = abstractArray->GetDataType();
        using H5Type = std::variant<AtomicType<float>, AtomicType<short>>;

        H5Type h5Type = [vtkDataType]() -> H5Type {
            switch (vtkDataType) {
                case VTK_FLOAT: return AtomicType<float> {};
                case VTK_SHORT: return AtomicType<short> {};
                default: throw std::runtime_error("vtk data type not supported");
            }
        }();

        std::visit([vtkDataType, &file, &arrayName, &dataSpace](auto type) {
            auto dataSet = file.createDataSet(arrayName, dataSpace, type);
            dataSet.createAttribute("vtkType", vtkDataType);
        }, h5Type);
    }

    HighFive::DataSpace const sampleIdsAttributeDataSpace { TotalNumberOfImages };
    file.createAttribute("sample ids", sampleIdsAttributeDataSpace, GetSampleIdDataType());

    std::vector<int> const imageExtent { image.GetExtent(), std::next(image.GetExtent(), 6) };
    std::vector<double> const imageSpacing { image.GetSpacing(), std::next(image.GetSpacing(), 3) };
    std::vector<double> const imageOrigin { image.GetOrigin(), std::next(image.GetOrigin(), 3) };

    file.createAttribute("extent", imageExtent);
    file.createAttribute("spacing", imageSpacing);
    file.createAttribute("origin", imageOrigin);

    file.createAttribute("number of images", TotalNumberOfImages);
}

auto HdfImageWriter::WriteImageBatch(HighFive::File& file) -> void {
    auto sampleIdsAttribute = file.getAttribute("sample ids");
    std::vector<SampleId> sampleIdsBuffer { sampleIdsAttribute.getSpace().getElementCount() };
    sampleIdsAttribute.read_raw<SampleId>(sampleIdsBuffer.data(), GetSampleIdDataType());
    auto batchSampleIds = std::views::transform(Batch, &BatchImage::Id);
    std::copy(batchSampleIds.begin(), batchSampleIds.end(),
              std::next(sampleIdsBuffer.begin(), NumberOfProcessedImages));
    std::span<SampleId> const sampleIds { sampleIdsBuffer.begin(),
                                          std::next(sampleIdsBuffer.begin(),
                                                    NumberOfProcessedImages + InputImages.size()) };

    std::vector<SampleId> sortedSampleIds { sampleIds.begin(), sampleIds.end() };
    std::sort(sortedSampleIds.begin(), sortedSampleIds.end());
    auto adjacentIt = std::adjacent_find(sortedSampleIds.begin(), sortedSampleIds.end());
    if (adjacentIt != sortedSampleIds.end())
        throw std::runtime_error("duplicate sample ids");

    sampleIdsAttribute.write_raw(sampleIdsBuffer.data(), GetSampleIdDataType());

    using ImageDataVectors = std::variant<std::vector<std::vector<float>>, std::vector<std::vector<short>>>;
    using ImagesDataVectors = std::vector<ImageDataVectors>;
    ImagesDataVectors imagesDataVectors { ArrayNames.size() };
    std::transform(ArrayNames.cbegin(), ArrayNames.cend(),
                   imagesDataVectors.begin(),
                   [this](std::string const& arrayName) -> ImageDataVectors {
        auto* pointData = InputImages.at(0).get().GetPointData();
        if (!pointData)
            throw std::runtime_error("point data must not be null");

        auto* abstractArray = pointData->GetAbstractArray(arrayName.data());
        if (!abstractArray)
            throw std::runtime_error("abstract array must not be null");

        int const vtkDataType = abstractArray->GetDataType();
        std::variant<vtkFloatArray*, vtkTypeInt16Array*> arrayPointerVariant
        = [vtkDataType, abstractArray]() -> std::variant<vtkFloatArray*, vtkTypeInt16Array*> {

            switch (vtkDataType) {
                case VTK_FLOAT: return vtkFloatArray::SafeDownCast(abstractArray);
                case VTK_SHORT: return vtkTypeInt16Array::SafeDownCast(abstractArray);
                default: throw std::runtime_error("vtk data type not supported");
            }
        }();

        size_t const numberOfTuples = std::visit([](auto* firstArray) { return firstArray->GetNumberOfTuples(); },
                                                 arrayPointerVariant);

        return std::visit([this, numberOfTuples, &arrayName](auto* firstArray) -> ImageDataVectors {
            using ArrayType = std::remove_pointer_t<decltype(firstArray)>;
            using ValueType = ArrayType::ValueType;

            std::vector<std::vector<ValueType>> imageDataVectors { InputImages.size() };
            std::ranges::transform(InputImages | std::views::transform(&std::reference_wrapper<vtkImageData>::get),
                                   imageDataVectors.begin(),
                                   [numberOfTuples, &arrayName](vtkImageData& image) -> std::vector<ValueType> {

                auto* pointData = image.GetPointData();
                if (!pointData)
                    throw std::runtime_error("point data must not be null");

                auto* abstractArray = pointData->GetAbstractArray(arrayName.data());
                if (!abstractArray)
                    throw std::runtime_error("abstract array must not be null");

                auto* array = ArrayType::SafeDownCast(abstractArray);

                auto* beginPointer = array->WritePointer(0, numberOfTuples);

                return std::vector<ValueType> { beginPointer, std::next(beginPointer, numberOfTuples) };
            });
            return imageDataVectors;
        }, arrayPointerVariant);
    });

    for (size_t i = 0; i < ArrayNames.size(); i++) {
        auto& arrayName = ArrayNames.at(i);
        auto& dataVector = imagesDataVectors.at(i);

        auto dataSet = file.getDataSet(ArrayNames.at(i));
        auto const dataSpaceDimensions = dataSet.getSpace().getDimensions();
        auto const firstIdx = NumberOfProcessedImages;
        auto const numberOfImages = InputImages.size();
        std::vector<size_t> const offset { firstIdx, 0 };
        std::vector<size_t> const counts { numberOfImages, dataSpaceDimensions.at(1) };

        std::visit([&dataSet, &offset, &counts](auto& data) {
            auto selection = dataSet.select(offset, counts);
            selection.write(data);
        }, dataVector);
    }

    NumberOfProcessedImages += InputImages.size();

    TruncateFileBeforeWrite = false;
}

auto HdfImageWriter::GetSampleIdDataType() noexcept -> HighFive::DataType const& {
    using HighFive::AtomicType;
    using HighFive::CompoundType;

    static const CompoundType sampleIdType {
            std::vector<CompoundType::member_def> {
                    CompoundType::member_def { "group id" , AtomicType<uint16_t>{}, 0 },
                    CompoundType::member_def { "state id" , AtomicType<uint16_t>{}, sizeof(uint16_t) }
            },
            sizeof(uint16_t) * 2
    };

    return sampleIdType;
}
