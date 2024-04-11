#pragma once

#include "CtStructure.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

#include "tracy/Tracy.hpp"

class BasicStructure;
class CtStructure;
class CombinedStructure;
class Pipeline;
class PipelineList;

struct BasicStructureData;
struct CombinedStructureData;

class QVariant;

enum class CtStructureTreeEventType { Add, Remove };
struct CtStructureTreeEvent {
    CtStructureTreeEventType Type;
    CtStructure& Structure;
};

class CtStructureTree : public vtkObject {
public:
    static CtStructureTree* New();
    vtkTypeMacro(CtStructureTree, vtkObject)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void SetPipelineList(PipelineList& pipelineList);

    void AddBasicStructure(BasicStructure& basicStructure, CombinedStructure* parent = nullptr);

    void AddBasicStructure(const BasicStructureData& basicStructureData, CombinedStructure* parent = nullptr);

    void CombineWithBasicStructure(BasicStructure& basicStructure, CombinedStructure& combinedStructure);

    void CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                                   const CombinedStructureData& combinedStructureData);

    void RefineWithBasicStructure(const BasicStructureData& newStructureData,
                                  const CombinedStructureData& combinedStructureData,
                                  BasicStructure& structureToRefine);

    void RemoveBasicStructure(BasicStructure& basicStructure);

    CtStructure* GetRoot() const;

//    inline void EvaluateAtPosition(const double x[3], CtStructure::Result& result);

    CtStructure::ModelingResult FunctionValueAndRadiodensity(const double x[3]);

    void SetData(CtStructure* ctStructure, const QVariant& data);

    void Iterate(const std::function<void(CtStructure&)>& f) const;

    CtStructureTree(const CtStructureTree&) = delete;
    void operator=(const CtStructureTree&) = delete;

protected:
    CtStructureTree() = default;
    ~CtStructureTree() override = default;

    bool CtStructureExists(const CtStructure& ctStructure);

    void EmitEvent(CtStructureTreeEvent event);

    vtkSmartPointer<CtStructure> Root;
    vtkWeakPointer<PipelineList> Pipelines;
};


