#pragma once

#include <vtkImageAlgorithm.h>

#include "HdfImageWriter.h"

namespace HighFive {
    class File;
}


class HdfImageReader : public vtkImageAlgorithm {
public:
    HdfImageReader(HdfImageReader const&) = delete;
    void operator=(HdfImageReader const&) = delete;

    static auto
    New() -> HdfImageReader*;

    vtkTypeMacro(HdfImageReader, vtkImageAlgorithm);

    virtual auto
    SetFilename(std::filesystem::path const& filename) noexcept -> void {
        if (Filename == filename)
            return;

        Filename = filename;

        Modified();
    }

    virtual auto
    SetArrayNames(std::vector<std::string>&& arrayNames) noexcept -> void {
        if (ArrayNames == arrayNames)
            return;

        ArrayNames = std::move(arrayNames);

        Modified();
    }

    using BatchImage = HdfImageWriter::BatchImage;
    using BatchImages = HdfImageWriter::BatchImages;

    auto
    ReadImageBatch(BatchImages& images) -> void;

    struct ValidationParameters {
        uint64_t NumberOfImages;
//        uint64_t ImageSize;
        std::vector<std::string> ArrayNames;
    };

    static auto
    Validate(std::filesystem::path const& file, ValidationParameters const& params) -> void;

protected:
    HdfImageReader() = default;
    ~HdfImageReader() override = default;

    auto FillInputPortInformation(int port, vtkInformation *info) -> int override;
    auto FillOutputPortInformation(int port, vtkInformation *info) -> int override;

private:
    std::filesystem::path Filename;
    std::vector<std::string> ArrayNames;
};
