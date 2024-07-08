#pragma once

#include <vtkImageAlgorithm.h>

#include "HdfImageWriter.h"


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

    using BatchImage = HdfImageWriter::BatchImage;
    using BatchImages = HdfImageWriter::BatchImages;

    auto
    ReadImageBatch(BatchImages& images) -> void;

protected:
    HdfImageReader() = default;
    ~HdfImageReader() override = default;

    auto FillInputPortInformation(int port, vtkInformation *info) -> int override;
    auto FillOutputPortInformation(int port, vtkInformation *info) -> int override;

private:
    auto
    ReadImageBatch1(HighFive::DataSet& dataSet) -> void;

    std::filesystem::path Filename;
    SampleId MaxSampleId {};
    BatchImages Batch {};
    uint16_t TotalNumberOfImages = 0;
    uint16_t ProcessedImages = 0;
    bool TruncateFileBeforeWrite = true;

    std::vector<std::reference_wrapper<vtkImageData>> InputImages;
};
