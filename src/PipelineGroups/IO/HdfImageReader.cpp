#include "HdfImageReader.h"

#include <vtkImageData.h>
#include <vtkObjectFactory.h>

#include <highfive/highfive.hpp>

#include <ranges>


vtkStandardNewMacro(HdfImageReader)

auto HdfImageReader::FillInputPortInformation(int /*port*/, vtkInformation* /*info*/) -> int {
    throw std::runtime_error("not implemented");
}

auto HdfImageReader::FillOutputPortInformation(int /*port*/, vtkInformation* /*info*/) -> int {
    throw std::runtime_error("not implemented");
}

auto HdfImageReader::ReadImageBatch(BatchImages& batchImages) -> void {
    if (batchImages.empty())
        return;

    if (Filename.empty() || !is_regular_file(Filename))
        throw std::runtime_error("invalid filename");

    auto file = HighFive::File(Filename.string(), HighFive::File::ReadOnly);
    auto dataSet = file.getDataSet("images");


    auto sampleIdsAttribute = dataSet.getAttribute("sample ids");
    std::vector<SampleId> sampleIds { sampleIdsAttribute.getSpace().getElementCount() };
    sampleIdsAttribute.read_raw<SampleId>(sampleIds.data(), HdfImageWriter::GetSampleIdDataType());

    auto imageExtentAttribute = dataSet.getAttribute("extent");
    auto imageSpacingAttribute = dataSet.getAttribute("spacing");
    auto imageOriginAttribute = dataSet.getAttribute("origin");

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
    std::transform(batchImages.cbegin(), batchImages.cend(), flatReadPile.begin(),
                   [&sampleIds](BatchImage const& batchImage) {

        SampleId const& sampleId = batchImage.Id;

        auto it = std::find(sampleIds.cbegin(), sampleIds.cend(), sampleId);
        if (it == sampleIds.cend())
            throw std::runtime_error("invalid sample id");

        uint16_t const idx = std::distance(sampleIds.cbegin(), it);

        return IdxSampleIdPair { idx, sampleId };
    });

    std::sort(flatReadPile.begin(), flatReadPile.end(), [](auto const& a, auto const& b) { return a.Idx < b.Idx; });

    ReadPile currentPile;
    for (int i = 0; i < flatReadPile.size() - 1; i++) {
        IdxSampleIdPair const& pair = flatReadPile.at(i);
        IdxSampleIdPair const& nextPair = flatReadPile.at(i + 1);

        currentPile.push_back(pair);

        bool const adjacent = nextPair.Idx - pair.Idx == 1;
        if ((!adjacent && i != 0) || i == flatReadPile.size() - 1) {
            readPiles.push_back(currentPile);
            currentPile.clear();
        }
    }

    struct PileHyperSlabPair {
        ReadPile Pile;
        HighFive::HyperSlab Slab;
    };
    auto const dataSpaceDimensions = dataSet.getSpace().getDimensions();
    std::vector<PileHyperSlabPair> pileHyperSlabPairs { readPiles.size() };
    std::transform(readPiles.cbegin(), readPiles.cend(), pileHyperSlabPairs.begin(),
                   [&dataSpaceDimensions, &dataSet](ReadPile const& pile) {
        auto const firstIdx = pile.at(0).Idx;
        auto const numberOfImages = pile.at(pile.size() - 1).Idx - firstIdx;
        std::vector<size_t> const startCoordinates { 0, 0, 0, firstIdx };
        std::vector<size_t> const counts { 4, 1 };
        std::vector<size_t> blockSize { dataSpaceDimensions.begin(), dataSpaceDimensions.end() };
        blockSize.push_back(numberOfImages);

        HighFive::RegularHyperSlab const regularSlab { startCoordinates, counts, {}, blockSize };
        HighFive::HyperSlab const hyperSlab { regularSlab };

        return PileHyperSlabPair { pile, hyperSlab };
    });

    for (auto const& pair : pileHyperSlabPairs) {
        size_t const imagesDataSize = batchImages.at(0).ImageData.GetNumberOfPoints() * pair.Pile.size();
        std::vector<float> imagesData {};
        imagesData.resize(imagesDataSize);

        auto selection = dataSet.select(pair.Slab);
        selection.read_raw(imagesData.data());
    }
}
