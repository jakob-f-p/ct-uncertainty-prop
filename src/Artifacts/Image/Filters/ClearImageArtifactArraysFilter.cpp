#include "ClearImageArtifactArraysFilter.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

vtkStandardNewMacro(ClearImageArtifactArraysFilter)

int ClearImageArtifactArraysFilter::RequestInformation(vtkInformation* request,
                                                       vtkInformationVector** inputVector,
                                                       vtkInformationVector* outputVector) {
    CopyInputArrayAttributesToOutput(request, inputVector, outputVector);

    outputVector->GetInformationObject(0)->Set(CAN_PRODUCE_SUB_EXTENT(), 1);

    return 1;
}

int ClearImageArtifactArraysFilter::RequestData(vtkInformation* request,
                                                vtkInformationVector** inputVector,
                                                vtkInformationVector* outputVector) {
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkPointData* inputPointData = input->GetPointData();
    inputPointData->CopyAllOff();
    inputPointData->CopyScalarsOn();

    vtkPointData* outputPointData = output->GetPointData();
    outputPointData->PassData(inputPointData);

    return 1;
}
