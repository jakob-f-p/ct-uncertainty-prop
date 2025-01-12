#include "NrrdCtDataSource.h"

#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkNew.h>
#include <vtkNrrdReader.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkTypeUInt8Array.h>

#include <cassert>
#include <ranges>
#include <span>

vtkStandardNewMacro(NrrdCtDataSource)

void NrrdCtDataSource::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "File: (" << Filename << ")\n";
}

void NrrdCtDataSource::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) {
    vtkImageData* data = vtkImageData::SafeDownCast(output);

    vtkNew<vtkNrrdReader> const nrrdReader;
    nrrdReader->SetFileName(absolute(Filename).string().c_str());
    nrrdReader->Update();
    data->ShallowCopy(nrrdReader->GetOutput());

    std::span<int, 3> const oldDimensions { data->GetDimensions(), 3 };
    std::array<int, 3> const newDimensions = GetDimensions();
    std::array<double, 3> dimRatios {};
    for (int i = 0; i < 3; ++i)
        dimRatios[i] = static_cast<double>(newDimensions[i]) / static_cast<double>(oldDimensions[i]);

    vtkNew<vtkImageResample> const imageResample;
    imageResample->SetMagnificationFactors(dimRatios.data());
    imageResample->SetInputData(data);
    imageResample->Update();

    data->ShallowCopy(imageResample->GetOutput());

    data->SetOrigin(GetOrigin().data());
    data->SetSpacing(GetSpacing().data());

    assert(([data, targetExtent = GetWholeExtent()]() -> bool {
        std::span<int, 6> const newExtent { data->GetExtent(), 6 };
        bool allSame = true;
        for (int i = 0; i < 6; ++i)
            allSame &= (newExtent[i] == targetExtent[i]);
        return allSame;
    })());

    auto* pointData = data->GetPointData();
    auto* dataArray = pointData->GetScalars();
    auto* uint8DataArray = vtkTypeUInt8Array::SafeDownCast(dataArray);
    if (!uint8DataArray)
        throw std::runtime_error("unsupported data type");

    vtkNew<vtkFloatArray> const floatArray;
    floatArray->DeepCopy(uint8DataArray);
    floatArray->SetName("Radiodensities");

    pointData->RemoveArray(0);
    pointData->AddArray(floatArray);
    pointData->SetActiveScalars("Radiodensities");
}
