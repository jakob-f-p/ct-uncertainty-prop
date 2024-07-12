#pragma once

#include <vtkStructuredPointsWriter.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

class vtkImageData;


class ImageScalarsWriter : public vtkStructuredPointsWriter {
public:
    ImageScalarsWriter(const ImageScalarsWriter&) = delete;
    void operator=(const ImageScalarsWriter&) = delete;

    static ImageScalarsWriter* New();
    vtkTypeMacro(ImageScalarsWriter, vtkDataWriter);

    void SetScalarsArrayName(std::string name);
    std::string GetScalarsArrayName();

protected:
    ImageScalarsWriter() = default;
    ~ImageScalarsWriter() override = default;

    vtkImageData* GetInput();

    void UpdateImageDataObjects();

    void WriteData() override;

    vtkSmartPointer<vtkImageData> FullImageData;
    vtkNew<vtkImageData> StrippedImageData;
    vtkMTimeType LastUpdated = 0;

    std::string ScalarsArrayName;
};
