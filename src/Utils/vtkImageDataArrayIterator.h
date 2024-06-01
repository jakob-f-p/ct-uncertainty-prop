#pragma once

#include "vtkImageIterator.h"


template<typename DType>
class vtkImageDataArrayIterator : public vtkImageIterator<DType> {
    using ExtentType = int[6];

public:
    vtkImageDataArrayIterator(vtkImageData* id, ExtentType ext, std::string const& arrayName);

    void Initialize(vtkImageData* id, ExtentType ext, std::string const& arrayName);
};
