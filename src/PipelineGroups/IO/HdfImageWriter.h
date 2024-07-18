#pragma once

#include "../Types.h"

#include <vtkWriter.h>

#include <filesystem>

namespace HighFive {
    class DataType;
    class File;
}
class vtkImageData;


class HdfImageWriter : public vtkWriter {
public:
    HdfImageWriter(const HdfImageWriter&) = delete;
    void operator=(const HdfImageWriter&) = delete;

    static auto
    New() -> HdfImageWriter*;

    vtkTypeMacro(HdfImageWriter, vtkWriter);

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

    struct BatchImage {
        SampleId Id;
        vtkImageData& ImageData;

        [[nodiscard]] auto
        operator== (BatchImage const& other) const noexcept -> bool {
            return Id == other.Id && &ImageData == &other.ImageData;
        }
    };
    using BatchImages = std::vector<BatchImage>;

    virtual auto
    SetBatch(BatchImages&& images) noexcept -> void;

    virtual auto
    SetTotalNumberOfImages(uint16_t totalNumberOfImages) -> void {
        if (TotalNumberOfImages == totalNumberOfImages)
            return;

        TotalNumberOfImages = totalNumberOfImages;

        Modified();
    }

    virtual auto
    SetTruncateFileBeforeWrite(bool truncate) noexcept -> void {
        if (TruncateFileBeforeWrite == truncate)
            return;

        TruncateFileBeforeWrite = truncate;

        Modified();
    }

    virtual auto
    GetTruncateFileBeforeWrite() const noexcept -> bool { return TruncateFileBeforeWrite; }

    auto
    Write() -> int override;

protected:
    HdfImageWriter();
    ~HdfImageWriter() override = default;

    auto
    WriteData() -> void override;

private:
    friend class HdfImageReader;

    auto
    InitializeFile(HighFive::File& file) -> void;

    auto
    WriteImageBatch(HighFive::File& file) -> void;

    [[nodiscard]] static auto
    GetSampleIdDataType() noexcept -> HighFive::DataType const&;

    std::filesystem::path Filename;
    std::vector<std::string> ArrayNames;
    SampleId MaxSampleId {};
    BatchImages Batch {};
    uint16_t TotalNumberOfImages = 0;
    uint16_t NumberOfProcessedImages = 0;
    bool TruncateFileBeforeWrite = true;

    std::vector<std::reference_wrapper<vtkImageData>> InputImages;
};
