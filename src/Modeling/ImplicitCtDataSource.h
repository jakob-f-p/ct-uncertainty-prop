#pragma once

#include "CtDataSource.h"

#include <array>

class CtStructureTree;


class ImplicitCtDataSource : public CtDataSource {
public:
    static ImplicitCtDataSource* New();
    vtkTypeMacro(ImplicitCtDataSource, CtDataSource);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void SetDataTree(CtStructureTree* ctStructureTree);

    ImplicitCtDataSource(const ImplicitCtDataSource&) = delete;
    void operator=(const ImplicitCtDataSource&) = delete;

protected:
    ImplicitCtDataSource() = default;
    ~ImplicitCtDataSource() override = default;

    void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

    struct SampleAlgorithm {
        ImplicitCtDataSource* Self;
        vtkImageData* VolumeData;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
        DoublePoint StartPoint;
        CtStructureTree* Tree;
        float* Radiodensities;
        float* FunctionValues;
        uint16_t* BasicStructureIds;

        SampleAlgorithm(ImplicitCtDataSource* self,
                        vtkImageData* volumeData,
                        CtStructureTree* tree,
                        float* radiodensities,
                        float* functionValues,
                        uint16_t* basicStructureIds);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    CtStructureTree* DataTree = nullptr;
};
