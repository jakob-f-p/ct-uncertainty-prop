#include "ImageScalarsWriter.h"

#include <vtkObjectFactory.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>

vtkStandardNewMacro(ImageScalarsWriter)

void ImageScalarsWriter::SetScalarsArrayName(std::string name) {
    ScalarsArrayName = std::move(name);
    Modified();
}

std::string ImageScalarsWriter::GetScalarsArrayName() {
    return ScalarsArrayName;
}

vtkImageData* ImageScalarsWriter::GetInput() {
    if (LastUpdated < GetMTime())
        UpdateImageDataObjects();

    return StrippedImageData;
}

void ImageScalarsWriter::UpdateImageDataObjects() {
    FullImageData = vtkStructuredPointsWriter::GetInput();

    StrippedImageData->CopyStructure(FullImageData);
    StrippedImageData->CopyAttributes(FullImageData);

    auto* cellData = StrippedImageData->GetCellData();
    for (int i = cellData->GetNumberOfArrays() - 1; i >= 0; i--)
        cellData->RemoveArray(i);

    auto* fieldData = StrippedImageData->GetFieldData();
    for (int i = fieldData->GetNumberOfArrays() - 1; i >= 0; i--)
        fieldData->RemoveArray(i);

    auto* pointData = StrippedImageData->GetPointData();
    pointData->SetActiveScalars(ScalarsArrayName.c_str());
    for (int i = pointData->GetNumberOfArrays() - 1; i >= 0; i--) {
        auto const* name = pointData->GetArray(i)->GetName();
        if (name != ScalarsArrayName)
            pointData->RemoveArray(i);
    }

    LastUpdated = GetMTime();
}

void ImageScalarsWriter::WriteData() {
    ostream* fp;
    vtkImageData* input = vtkImageData::SafeDownCast(this->GetInput());
    int dim[3];
    int* ext;
    double spacing[3], origin[3];

    vtkDebugMacro(<< "Writing vtk structured points...");

    if (!(fp = this->OpenVTKFile()) || !this->WriteHeader(fp)) {
        if (fp) {
            vtkErrorMacro("Ran out of disk space; deleting file: " << this->FileName);
            this->CloseVTKFile(fp);
            _unlink(this->FileName);
        }
        return;
    }
    *fp << "DATASET STRUCTURED_POINTS\n";

    if (!this->WriteDataSetData(fp, input)) {
        vtkErrorMacro("Ran out of disk space; deleting file: " << this->FileName);
        this->CloseVTKFile(fp);
        _unlink(this->FileName);
        return;
    }

    if (this->WriteExtent) {
        int extent[6];
        input->GetExtent(extent);
        *fp << "EXTENT " << extent[0] << " " << extent[1] << " " << extent[2] << " " << extent[3] << " "
            << extent[4] << " " << extent[5] << "\n";
    } else {
        input->GetDimensions(dim);
        *fp << "DIMENSIONS " << dim[0] << " " << dim[1] << " " << dim[2] << "\n";
    }

    input->GetSpacing(spacing);
    *fp << "SPACING " << spacing[0] << " " << spacing[1] << " " << spacing[2] << "\n";

    input->GetOrigin(origin);
    if (this->WriteExtent) {
        *fp << "ORIGIN " << origin[0] << " " << origin[1] << " " << origin[2] << "\n";
    } else {
        // Do the electric slide. Move origin to min corner of extent.
        // The alternative is to change the format to include an extent instead of dimensions.
        ext = input->GetExtent();
        origin[0] += ext[0] * spacing[0];
        origin[1] += ext[2] * spacing[1];
        origin[2] += ext[4] * spacing[2];
        *fp << "ORIGIN " << origin[0] << " " << origin[1] << " " << origin[2] << "\n";
    }

    if (!this->WriteCellData(fp, input)) {
        vtkErrorMacro("Ran out of disk space; deleting file: " << this->FileName);
        this->CloseVTKFile(fp);
        _unlink(this->FileName);
        return;
    }

    if (!this->WritePointData(fp, input)) {
        vtkErrorMacro("Ran out of disk space; deleting file: " << this->FileName);
        this->CloseVTKFile(fp);
        _unlink(this->FileName);
        return;
    }

    this->CloseVTKFile(fp);
}
