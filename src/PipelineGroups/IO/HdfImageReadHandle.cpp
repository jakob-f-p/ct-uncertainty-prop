#include "HdfImageReadHandle.h"

#include "HdfImageReader.h"

#include <vtkImageData.h>


HdfImageReadHandle::HdfImageReadHandle(std::filesystem::path const& hdfFilePath, SampleId sampleId) noexcept :
        Filename(hdfFilePath),
        SId(sampleId) {}

auto HdfImageReadHandle::Read() const noexcept -> vtkSmartPointer<vtkImageData> {
    vtkNew<HdfImageReader> reader;
    reader->SetFilename(Filename);
    reader->SetArrayNames({ "Radiodensities" });

    vtkNew<vtkImageData> image;
    HdfImageWriter::BatchImages batchImages { { SId, *image } };

    reader->ReadImageBatch(batchImages);

    return image;
}
