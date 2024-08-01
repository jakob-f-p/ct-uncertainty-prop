#include "ImplicitCtDataSource.h"

#include "CtStructureTree.h"
#include "../Utils/ImageDataUtils.h"

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTypeUInt16Array.h>

vtkStandardNewMacro(ImplicitCtDataSource)

void ImplicitCtDataSource::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Implicit CT Structures: (" << DataTree << ")\n";
}

auto ImplicitCtDataSource::GetMTime() -> vtkMTimeType {
    vtkMTimeType const mTime = Superclass::GetMTime();
    vtkMTimeType const treeMTime = DataTree ? DataTree->GetMTime() : 0;

    return std::max({ mTime, treeMTime });
}

void ImplicitCtDataSource::SetDataTree(CtStructureTree* ctStructureTree) { DataTree = ctStructureTree; }

void ImplicitCtDataSource::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) {
    vtkImageData* data = vtkImageData::SafeDownCast(output);
    int* updateExtent = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    data->SetExtent(updateExtent);
    vtkIdType const numberOfPoints = data->GetNumberOfPoints();

    vtkFloatArray* radiodensityArray = vtkFloatArray::New();
    radiodensityArray->SetNumberOfComponents(1);
    radiodensityArray->SetName("Radiodensities");
    radiodensityArray->SetNumberOfTuples(numberOfPoints);
    data->GetPointData()->SetScalars(radiodensityArray);
    radiodensityArray->Delete();
    float* radiodensities = radiodensityArray->WritePointer(0, numberOfPoints);

    vtkFloatArray* functionValueArray = vtkFloatArray::New();
    functionValueArray->SetNumberOfComponents(1);
    functionValueArray->SetName("FunctionValues");
    functionValueArray->SetNumberOfTuples(numberOfPoints);
    data->GetPointData()->AddArray(functionValueArray);
    functionValueArray->Delete();
    float* functionValues = functionValueArray->WritePointer(0, numberOfPoints);

    vtkTypeUInt16Array* basicStructureIdArray = vtkTypeUInt16Array::New();
    basicStructureIdArray->SetNumberOfComponents(1);
    basicStructureIdArray->SetName("BasicStructureIds");
    basicStructureIdArray->SetNumberOfTuples(numberOfPoints);
    data->GetPointData()->AddArray(basicStructureIdArray);
    basicStructureIdArray->Delete();
    uint16_t* basicStructureIds = basicStructureIdArray->WritePointer(0, numberOfPoints);

    if (!DataTree->HasRoot()) {
        vtkDebugMacro("Ct data tree has 0 nodes. Cannot evaluate");
        return;
    }

    SampleAlgorithm sampleAlgorithm { this, data, DataTree, radiodensities, functionValues, basicStructureIds };
    vtkSMPTools::For(0, numberOfPoints, sampleAlgorithm);
}

ImplicitCtDataSource::SampleAlgorithm::SampleAlgorithm(ImplicitCtDataSource* self,
                                                       vtkImageData* volumeData,
                                                       CtStructureTree* tree,
                                                       float* radiodensities,
                                                       float* functionValues,
                                                       uint16_t* basicStructureIds) :
        Self(self),
        VolumeData(volumeData),
        Spacing(self->GetSpacing()),
        UpdateDims([this]() {
            std::array<int, 3> updateDims {};
            std::copy(VolumeData->GetDimensions(), std::next(VolumeData->GetDimensions(), 3), updateDims.begin());
            return updateDims;
        }()),
        StartPoint([this]() {
            Point startPoint {};
            VolumeData->GetPoint(0, startPoint.data());
            return startPoint;
        }()),
        Tree(tree),
        Radiodensities(radiodensities),
        FunctionValues(functionValues),
        BasicStructureIds(basicStructureIds) {}

void ImplicitCtDataSource::SampleAlgorithm::operator()(vtkIdType pointId, vtkIdType endPointId) const {
    Self->CheckAbort();

    if (Self->GetAbortOutput())
        return;

    std::array<int, 3> const startPointCoordinates = PointIdToDimensionCoordinates(pointId, UpdateDims);
    std::array<int, 3> const endPointCoordinates = PointIdToDimensionCoordinates(endPointId, UpdateDims);
    auto [ x1, y1, z1 ] = startPointCoordinates;
    auto lastValidPointCoordinates = GetDecrementedCoordinates(endPointCoordinates, UpdateDims);
    auto [ x2, y2, z2 ] = lastValidPointCoordinates;

    Point point = StartPoint;
    point[0] += x1 * Spacing[0];
    point[1] += y1 * Spacing[1];
    point[2] += z1 * Spacing[2];

    for (vtkIdType z = z1; z <= z2; z++) {
        vtkIdType y = 0;
        if (z == z1)
            y = y1;

        vtkIdType yEnd = UpdateDims[1] - 1;
        if (z == z2)
            yEnd = y2;

        for (; y <= yEnd; y++) {
            vtkIdType x = 0;
            if (z == z1 && y == y1)
                x = x1;

            vtkIdType xEnd = UpdateDims[0] - 1;
            if (z == z2 && y == y2)
                xEnd = x2;

            for (; x <= xEnd; x++) {
                CtStructureTree::ModelingResult const result = Tree->FunctionValueAndRadiodensity(point);

                bool const pointIsWithinStructure = result.FunctionValue < 0;
                FunctionValues[pointId] = result.FunctionValue;
                Radiodensities[pointId] = pointIsWithinStructure
                                          ? result.Radiodensity
                                          : -1000.0f;
                BasicStructureIds[pointId] = pointIsWithinStructure
                                             ? result.BasicCtStructureId
                                             : 0;
                pointId++;

                point[0] += Spacing[0];
            }

            point[0] = StartPoint[0];
            point[1] += Spacing[1];
        }

        point[0] = StartPoint[0];
        point[1] = StartPoint[1];
        point[2] += Spacing[2];
    }
}
