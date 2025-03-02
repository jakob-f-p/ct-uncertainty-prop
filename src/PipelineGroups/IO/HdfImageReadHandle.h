#pragma once

#include "../Types.h"

#include <vtkSmartPointer.h>

#include <filesystem>

class vtkImageData;


class HdfImageReadHandle {
public:
    HdfImageReadHandle(std::filesystem::path const& hdfFilePath, SampleId sampleId) noexcept;

    [[nodiscard]] auto
    Read(std::vector<std::string>&& arrayNames = { "Radiodensities", "Segmentation Mask" })
    const noexcept -> vtkSmartPointer<vtkImageData>;

private:
    std::filesystem::path Filename;
    SampleId SId;
};
