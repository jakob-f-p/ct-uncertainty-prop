#pragma once

#include "CombinedStructure.h"
#include "tracy/Tracy.hpp"

#include <vtkObject.h>

struct BasicStructureDetails;

class CtStructureTree : public vtkObject {
public:
    static CtStructureTree* New();
    vtkTypeMacro(CtStructureTree, vtkObject)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void AddBasicStructure(BasicStructure& basicStructure,
                           CombinedStructure* parent = nullptr);

    void AddBasicStructure(const BasicStructureDetails& basicStructureDetails,
                           CombinedStructure* parent = nullptr);

    void CombineWithBasicStructure(BasicStructure& basicStructure,
                                   CombinedStructure::OperatorType operatorType);

    void CombineWithBasicStructure(BasicStructureDetails& basicStructureDetails);

    void RefineWithBasicStructure(const BasicStructureDetails& newStructureDetails,
                                  BasicStructure& structureToRefine,
                                  CombinedStructure::OperatorType operatorType = CombinedStructure::OperatorType::UNION);

    void RemoveBasicStructure(BasicStructure& basicStructure);

    CtStructure* GetRoot() const;

    inline void EvaluateAtPosition(const double x[3], CtStructure::Result& result);

    inline CtStructure::ModelingResult FunctionValueAndRadiodensity(const double x[3]);

    void SetData(CtStructure* ctStructure, const QVariant& data);

    void DeepCopy(CtStructureTree* source);

    CtStructureTree(const CtStructureTree&) = delete;
    void operator=(const CtStructureTree&) = delete;

protected:
    CtStructureTree();
    ~CtStructureTree() override;

    CtStructure* Root;

private:
    bool CtStructureExists(const CtStructure& ctStructure);
};

void CtStructureTree::EvaluateAtPosition(const double x[3], CtStructure::Result &result) {
    if (!Root) {
        qWarning("Tree does not have a root. Cannot evaluate");
        return;
    }

    return Root->EvaluateAtPosition(x, result);
}

inline CtStructure::ModelingResult CtStructureTree::FunctionValueAndRadiodensity(const double x[3]) {
    if (!Root) {
        qWarning("Tree does not have a root. Cannot evaluate");
        return {};
    }

    return Root->EvaluateImplicitModel(x);
}
