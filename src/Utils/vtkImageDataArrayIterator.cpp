#include "vtkImageDataArrayIterator.h"

#include "vtkImageData.h"
#include "vtkPointData.h"

#include <format>


template<typename DType>
vtkImageDataArrayIterator<DType>::vtkImageDataArrayIterator(vtkImageData* id,
                                                            ExtentType ext,
                                                            std::string const& arrayName) :
        vtkImageIterator<DType>() {

    Initialize(id, ext, arrayName);
}

template<typename DType>
void vtkImageDataArrayIterator<DType>::Initialize(vtkImageData* id, ExtentType ext, std::string const& arrayName) {
    auto* pointData = id->GetPointData();
    auto* array = pointData->GetArray(arrayName.c_str());
    if (!array)
        throw std::runtime_error(std::format("No array named '{}' exists", arrayName));

    std::array startCoordinates = { ext[0], ext[2], ext[4] };
    std::array endCoordinates =   { ext[1], ext[3], ext[5] };
    this->Pointer = static_cast<DType*>(id->GetArrayPointer(array, startCoordinates.data()));

    id->GetIncrements(this->Increments);
    id->GetContinuousIncrements(
            ext, this->ContinuousIncrements[0], this->ContinuousIncrements[1], this->ContinuousIncrements[2]);
    this->EndPointer = static_cast<DType*>(id->GetArrayPointer(array, endCoordinates.data())) + this->Increments[0];

    if (ext[1] < ext[0] || ext[3] < ext[2] || ext[5] < ext[4])
        this->EndPointer = this->Pointer;

    this->SpanEndPointer = this->Pointer + this->Increments[0] * (ext[1] - ext[0] + 1);
    this->SliceEndPointer = this->Pointer + this->Increments[1] * (ext[3] - ext[2] + 1);
}

template class vtkImageDataArrayIterator<vtkTypeInt8>;
template class vtkImageDataArrayIterator<vtkTypeUInt8>;
template class vtkImageDataArrayIterator<vtkTypeInt16>;
template class vtkImageDataArrayIterator<vtkTypeUInt16>;
template class vtkImageDataArrayIterator<vtkTypeUInt32>;
template class vtkImageDataArrayIterator<vtkTypeInt32>;
template class vtkImageDataArrayIterator<vtkTypeUInt64>;
template class vtkImageDataArrayIterator<vtkTypeInt64>;
template class vtkImageDataArrayIterator<vtkTypeFloat32>;
template class vtkImageDataArrayIterator<vtkTypeFloat64>;

template class vtkImageDataArrayIterator<char>;
template class vtkImageDataArrayIterator<long>;
template class vtkImageDataArrayIterator<unsigned long>;
