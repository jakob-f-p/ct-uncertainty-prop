#include "HdfImageWriter.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkTypeUInt16Array.h>

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

    unsigned int const openFlags = TruncateFileBeforeWrite ? HighFive::File::Truncate : HighFive::File::Excl;
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
    std::vector<size_t> dataSpaceDimensions { imageDimensions.begin(), imageDimensions.end() };
    dataSpaceDimensions.push_back(TotalNumberOfImages);
    HighFive::DataSpace const dataSpace { dataSpaceDimensions };
    for (auto const& arrayName : ArrayNames) {
        auto* pointData = image.GetPointData();
        if (!pointData)
            throw std::runtime_error("point data must not be null");

        auto* abstractArray = pointData->GetAbstractArray(arrayName.data());
        if (!abstractArray)
            throw std::runtime_error("abstract array must not be null");

        int const vtkDataType = abstractArray->GetDataType();
        using H5Type = std::variant<AtomicType<float>, AtomicType<unsigned short>>;

        H5Type h5Type = [vtkDataType]() -> H5Type {
            switch (vtkDataType) {
                case VTK_FLOAT:          return AtomicType<float> {};
                case VTK_UNSIGNED_SHORT: return AtomicType<unsigned short> {};
                default: throw std::runtime_error("vtk data type not supported");
            }
        }();

        std::visit([&file, &arrayName, &dataSpace](auto type) { file.createDataSet(arrayName, dataSpace, type); },
                   h5Type);
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
    std::vector<SampleId> sampleIds { sampleIdsAttribute.getSpace().getElementCount() };
    sampleIdsAttribute.read_raw<SampleId>(sampleIds.data(), GetSampleIdDataType());
    auto batchSampleIds = std::views::transform(Batch, &BatchImage::Id);
    std::copy(batchSampleIds.begin(), batchSampleIds.end(), std::next(sampleIds.begin(), NumberOfProcessedImages));

    std::vector<SampleId> sortedSampleIds { sampleIds.cbegin(), sampleIds.cend() };
    std::sort(sortedSampleIds.begin(), sortedSampleIds.end());
    auto adjacentIt = std::adjacent_find(sortedSampleIds.begin(), sortedSampleIds.end());
    if (adjacentIt != sortedSampleIds.end())
        throw std::runtime_error("duplicate sample ids");

    sampleIdsAttribute.write_raw(sampleIds.data(), GetSampleIdDataType());

    using ImageDataSpans = std::variant<std::vector<std::span<float>>, std::vector<std::span<unsigned short>>>;
    using ImagesDataSpans = std::vector<ImageDataSpans>;
    ImagesDataSpans imagesDataSpans { ArrayNames.size() };
    std::transform(ArrayNames.cbegin(), ArrayNames.cend(), imagesDataSpans.begin(),
                   [this](std::string const& arrayName) -> ImageDataSpans {
        auto* pointData = InputImages.at(0).get().GetPointData();
        if (!pointData)
            throw std::runtime_error("point data must not be null");

        auto* abstractArray = pointData->GetAbstractArray(arrayName.data());
        if (!abstractArray)
            throw std::runtime_error("abstract array must not be null");

        int const vtkDataType = abstractArray->GetDataType();
        std::variant<vtkFloatArray*, vtkTypeUInt16Array*> arrayPointerVariant
                = [vtkDataType, abstractArray]() -> std::variant<vtkFloatArray*, vtkTypeUInt16Array*> {

            switch (vtkDataType) {
                case VTK_FLOAT:          return vtkFloatArray::SafeDownCast(abstractArray);
                case VTK_UNSIGNED_SHORT: return vtkTypeUInt16Array::SafeDownCast(abstractArray);
                default: throw std::runtime_error("vtk data type not supported");
            }
        }();

        size_t const numberOfTuples = std::visit([](auto* firstArray) { return firstArray->GetNumberOfTuples(); },
                                              arrayPointerVariant);

        return std::visit([this, numberOfTuples, &arrayName](auto* firstArray) -> ImageDataSpans {
            using ArrayType = std::remove_pointer_t<decltype(firstArray)>;
            using ValueType = ArrayType::ValueType;

            std::vector<std::span<ValueType>> imageDataSpans { InputImages.size() };
            std::ranges::transform(InputImages | std::views::transform(&std::reference_wrapper<vtkImageData>::get),
                                   imageDataSpans.begin(),
                                   [numberOfTuples, &arrayName](vtkImageData& image) -> std::span<ValueType> {

                auto* pointData = image.GetPointData();
                if (!pointData)
                    throw std::runtime_error("point data must not be null");

                auto* abstractArray = pointData->GetAbstractArray(arrayName.data());
                if (!abstractArray)
                    throw std::runtime_error("abstract array must not be null");

                auto* array = ArrayType::SafeDownCast(abstractArray);

                auto* beginPointer = array->WritePointer(0, numberOfTuples);

                return std::span<ValueType> { beginPointer, std::next(beginPointer, numberOfTuples) };
            });

            return imageDataSpans;
        }, arrayPointerVariant);
    });

    using VectorVariant = std::variant<std::vector<float>, std::vector<unsigned short>>;
    std::vector<VectorVariant> imagesDataVector { ArrayNames.size() };
    uint32_t const totalImageSize = InputImages.at(0).get().GetNumberOfPoints() * TotalNumberOfImages;
    std::transform(imagesDataSpans.cbegin(), imagesDataSpans.cend(),
                   imagesDataVector.begin(),
                   [totalImageSize](ImageDataSpans const& imageDataSpans) -> VectorVariant {

        return std::visit([totalImageSize](auto const& spans) -> VectorVariant {
            using SpanVectorType = std::remove_reference_t<std::remove_cv_t<decltype(spans)>>;
            using SpanType = SpanVectorType::value_type;
            using ValueType = SpanType::value_type;

            std::vector<ValueType> imagesData {};
            imagesData.reserve(totalImageSize);
            for (auto const& imageDataSpan : spans)
                std::copy(imageDataSpan.begin(), imageDataSpan.end(), std::back_inserter(imagesData));

            return imagesData;
        }, imageDataSpans);
    });

    std::vector<HighFive::HyperSlab> hyperSlabs { ArrayNames.size() };
    std::transform(ArrayNames.cbegin(), ArrayNames.cend(),
                   hyperSlabs.begin(),
                   [this, &file](std::string const& dataSetName) {
        auto dataSet = file.getDataSet(dataSetName);
        auto const dataSpaceDimensions = dataSet.getSpace().getDimensions();

        auto const firstIdx = NumberOfProcessedImages;
        auto const numberOfImages = TotalNumberOfImages - NumberOfProcessedImages;
        std::vector<size_t> const startCoordinates { 0, 0, 0, firstIdx };
        std::vector<size_t> const counts { 4, 1 };
        std::vector<size_t> blockSize { dataSpaceDimensions.begin(), dataSpaceDimensions.end() };
        blockSize.push_back(numberOfImages);

        HighFive::RegularHyperSlab const regularSlab { startCoordinates, counts, {}, blockSize };

        return HighFive::HyperSlab { regularSlab };
    });

    for (size_t i = 0; i < ArrayNames.size(); i++) {
        auto dataSet = file.getDataSet(ArrayNames.at(i));
        auto& dataVector = imagesDataVector.at(i);
        auto& hyperSlab = hyperSlabs.at(i);

        std::visit([&dataSet, &hyperSlab](auto& data) {
            auto selection = dataSet.select(hyperSlab);
            selection.write_raw(data.data());
        }, dataVector);
    }

    NumberOfProcessedImages += InputImages.size();
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
