#include "HdfImageReader.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkTypeInt16Array.h>

#include <highfive/highfive.hpp>

#include <ranges>
#include <variant>


vtkStandardNewMacro(HdfImageReader)

auto HdfImageReader::FillInputPortInformation(int /*port*/, vtkInformation* /*info*/) -> int {
    throw std::runtime_error("not implemented");
}

auto HdfImageReader::FillOutputPortInformation(int /*port*/, vtkInformation* /*info*/) -> int {
    throw std::runtime_error("not implemented");
}

auto HdfImageReader::ReadImageBatch(BatchImages& batchImages) -> void {
    if (batchImages.empty())
        throw std::runtime_error("batch images must not be empty");

    if (ArrayNames.empty())
        throw std::runtime_error("array names must not be empty");

    if (Filename.empty() || !is_regular_file(Filename))
        throw std::runtime_error("invalid filename");


    auto file = HighFive::File(Filename.string(), HighFive::File::ReadOnly);


    auto sampleIdsAttribute = file.getAttribute("sample ids");
    std::vector<SampleId> sampleIds { sampleIdsAttribute.getSpace().getElementCount() };
    sampleIdsAttribute.read_raw<SampleId>(sampleIds.data(), HdfImageWriter::GetSampleIdDataType());

    auto imageExtentAttribute = file.getAttribute("extent");
    auto imageSpacingAttribute = file.getAttribute("spacing");
    auto imageOriginAttribute = file.getAttribute("origin");

    std::array<int, 6> imageExtent {};
    std::array<double, 3> imageSpacing {};
    std::array<double, 3> imageOrigin {};
    imageExtentAttribute.read(imageExtent);
    imageSpacingAttribute.read(imageSpacing);
    imageOriginAttribute.read(imageOrigin);

    for (auto& batchImage : batchImages) {
        auto& image = batchImage.ImageData;

        image.SetExtent(imageExtent.data());
        image.SetSpacing(imageSpacing.data());
        image.SetOrigin(imageOrigin.data());
    }

    struct IdxSampleIdPair {
        uint16_t Idx;
        SampleId SId;
    };
    using ReadPile = std::vector<IdxSampleIdPair>;

    std::vector<ReadPile> readPiles;
    ReadPile flatReadPile { batchImages.size() };
    std::transform(batchImages.cbegin(), batchImages.cend(),
                   flatReadPile.begin(),
                   [&sampleIds](BatchImage const& batchImage) {

        SampleId const& sampleId = batchImage.Id;

        auto it = std::find(sampleIds.cbegin(), sampleIds.cend(), sampleId);
        if (it == sampleIds.cend())
            throw std::runtime_error("invalid sample id");

        uint16_t const idx = std::distance(sampleIds.cbegin(), it);

        return IdxSampleIdPair { idx, sampleId };
    });

    ReadPile currentPile;
    for (int i = 0; i < flatReadPile.size(); i++) {
        IdxSampleIdPair const& pair = flatReadPile.at(i);

        if (i != 0 && pair.Idx - flatReadPile.at(i - 1).Idx != 1) {  // not first and not adjacent
            readPiles.push_back(currentPile);
            currentPile.clear();
        }

        currentPile.push_back(pair);

        if (i == flatReadPile.size() - 1) {  // last
            readPiles.push_back(currentPile);
            currentPile.clear();
        }
    }

    std::array<uint64_t, 3> imageDimensions {};
    for (int i = 0; i < imageDimensions.size(); i++)
        imageDimensions.at(i) = imageExtent.at(2 * i + 1) - imageExtent.at(2 * i) + 1;
    uint64_t const imageNumberOfPoints = std::reduce(imageDimensions.cbegin(), imageDimensions.cend(),
                                                     1, std::multiplies{});

    using PileDataVectorsVariant = std::variant<std::vector<std::vector<std::vector<float>>>,
                                                std::vector<std::vector<std::vector<short>>>>;
    std::vector<PileDataVectorsVariant> pileDataVectors { ArrayNames.size() };
    std::transform(ArrayNames.cbegin(), ArrayNames.cend(),
                   pileDataVectors.begin(),
                   [&readPiles, &file, imageNumberOfPoints](std::string const& arrayName) -> PileDataVectorsVariant {

        auto const dataSet = file.getDataSet(arrayName);
        auto const vtkTypeAttribute = dataSet.getAttribute("vtkType");
        auto const vtkDataType = vtkTypeAttribute.read<int>();

        std::variant<float, short> dataTypeHolderVariant = [vtkDataType]() -> std::variant<float, short> {
            switch (vtkDataType) {
                case VTK_FLOAT: return static_cast<float>(0);
                case VTK_SHORT: return static_cast<short>(0);
                default: throw std::runtime_error("vtk data type not supported");
            }
        }();

        return std::visit([&readPiles, &dataSet, imageNumberOfPoints](auto dataTypeHolder) -> PileDataVectorsVariant {
            using ValueType = decltype(dataTypeHolder);

            std::vector<std::vector<std::vector<ValueType>>> pileDataVectors {};
            pileDataVectors.reserve(readPiles.size());

            for (auto const& pile : readPiles) {
                std::vector<std::vector<ValueType>> dataVector {
                    pile.size(),
                    std::vector<ValueType> { imageNumberOfPoints,
                                             std::allocator<std::vector<ValueType>> {} }
                };
                size_t const firstIdx = pile.at(0).Idx;
                size_t const numberOfImages = pile.at(pile.size() - 1).Idx - firstIdx + 1;
                std::vector<size_t> const offset { firstIdx, 0 };
                std::vector<size_t> const counts { numberOfImages, imageNumberOfPoints };

                auto selection = dataSet.select(offset, counts);
                selection.read(dataVector);

                pileDataVectors.emplace_back(std::move(dataVector));
            }

            return pileDataVectors;
        }, dataTypeHolderVariant);
    });

    for (int i = 0; i < ArrayNames.size(); i++) {
        auto const& arrayName = ArrayNames.at(i);

        std::visit([&batchImages, &arrayName, imageNumberOfPoints](auto& pileDataVector) {
            using DataVectorType = std::remove_reference_t<std::remove_cv_t<decltype(pileDataVector)>>;
            using ValueType = DataVectorType::value_type::value_type::value_type;
            using VtkArrayType = std::conditional_t<std::is_same_v<ValueType, float>,
                    vtkFloatArray,
                    std::conditional_t<std::is_same_v<ValueType, short>,
                            vtkTypeInt16Array,
                            nullptr_t>>;

            auto imageVectorsViewIt = std::ranges::views::join(pileDataVector).begin();
            for (int j = 0; j < batchImages.size(); j++, imageVectorsViewIt++) {
                std::vector<ValueType> const& imageVector = *imageVectorsViewIt;
                auto& image = batchImages.at(j).ImageData;

                auto* pointData = image.GetPointData();
                if (!pointData)
                    throw std::runtime_error("point data must not be null");

                if (pointData->HasArray(arrayName.data()))
                    pointData->RemoveArray(arrayName.data());

                vtkNew<VtkArrayType> dataArray;
                dataArray->SetNumberOfComponents(1);
                dataArray->SetName(arrayName.data());
                dataArray->SetNumberOfTuples(imageNumberOfPoints);
                ValueType* writePointer = dataArray->WritePointer(0, imageNumberOfPoints);
                std::copy(imageVector.cbegin(), imageVector.cend(), writePointer);
                pointData->AddArray(dataArray);
                pointData->SetActiveScalars(arrayName.data());
            }
        }, pileDataVectors.at(i));
    }

    for (auto& batchImage : batchImages)
        batchImage.ImageData.GetPointData()->SetActiveScalars(ArrayNames.at(0).c_str());
}

auto HdfImageReader::Validate(std::filesystem::path const& filePath,
                              HdfImageReader::ValidationParameters const& params) -> void {
    if (params.ArrayNames.empty()
//        || params.ImageSize == 0
        || params.NumberOfImages == 0)
        throw std::runtime_error("invalid validation parameters");

    if (filePath.empty() || !is_regular_file(filePath))
        throw std::runtime_error("invalid filename");

    auto file = HighFive::File(filePath.string(), HighFive::File::ReadOnly);

    auto numberOfImagesAttribute = file.getAttribute("number of images");
    auto const numberOfImages = numberOfImagesAttribute.read<uint16_t>();
    if (numberOfImages != params.NumberOfImages)
        throw std::runtime_error("given file contains invalid number of images");

    auto imageExtentAttribute = file.getAttribute("extent");
    std::array<int, 6> imageExtent {};
    imageExtentAttribute.read(imageExtent);
    std::array<uint64_t, 3> imageDimensions {};
    for (int i = 0; i < imageDimensions.size(); i++)
        imageDimensions.at(i) = imageExtent.at(2 * i + 1) - imageExtent.at(2 * i) + 1;
    uint64_t const imageNumberOfPoints = std::reduce(imageDimensions.cbegin(), imageDimensions.cend(),
                                                     1, std::multiplies{});
//    if (imageNumberOfPoints != params.ImageSize)
//        throw std::runtime_error("given file contains images with invalid size");


    auto const objectNames = file.listObjectNames();
    uint16_t numberOfDataSets = 0;
    bool allDataSetsFound = true;
    for (auto const& name : objectNames) {
        try {
            file.getDataSet(name);

            if (std::find(params.ArrayNames.begin(), params.ArrayNames.end(), name) == params.ArrayNames.cend())
                allDataSetsFound = false;

            numberOfDataSets++;
        } catch (std::exception const& e) { /*ignore*/ }
    }
    if (!allDataSetsFound)
        throw std::runtime_error("given file does not contain datasets for all arrays");
}

