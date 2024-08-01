#include "NrrdCtDataSource.h"

#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkNrrdReader.h>
#include <span>
#include <cassert>

vtkStandardNewMacro(NrrdCtDataSource)

void NrrdCtDataSource::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "File: (" << Filename << ")\n";
}

void NrrdCtDataSource::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) {
    vtkImageData* data = vtkImageData::SafeDownCast(output);

    vtkNew<vtkNrrdReader> nrrdReader;
    nrrdReader->SetFileName(absolute(Filename).string().c_str());
    nrrdReader->Update();
    data->ShallowCopy(nrrdReader->GetOutput());
    data->Print(std::cout);

    std::span<int, 3> const oldDimensions { data->GetDimensions(), 3 };
    std::array<int, 3> const newDimensions = GetDimensions();
    std::array<double, 3> dimRatios {};
    for (int i = 0; i < 3; ++i)
        dimRatios[i] = static_cast<double>(newDimensions[i]) / static_cast<double>(oldDimensions[i]);

    vtkNew<vtkImageResample> imageResample;
    imageResample->SetMagnificationFactors(dimRatios.data());
    imageResample->SetInputData(data);
    imageResample->Update();

    data->ShallowCopy(imageResample->GetOutput());

    data->SetOrigin(GetOrigin().data());
    data->SetSpacing(GetSpacing().data());

    auto const targetExtent = GetWholeExtent();
    std::span<int, 6> const newExtent { data->GetExtent(), 6 };
    for (int i = 0; i < 6; ++i)
        assert(newExtent[i] == targetExtent[i]);
}
