#pragma once

#include "ImplicitStructureCombination.h"
#include "tracy/Tracy.hpp"

class CtDataCsgTree : public vtkObject {
public:
    static CtDataCsgTree* New();
    vtkTypeMacro(CtDataCsgTree, vtkObject)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void AddImplicitCtStructure(ImplicitCtStructure& implicitCtStructure,
                                ImplicitStructureCombination* parent = nullptr);

    void AddImplicitCtStructure(const ImplicitCtStructureDetails& implicitCtStructureDetails,
                                ImplicitStructureCombination* parent = nullptr);

    void CombineWithImplicitCtStructure(ImplicitCtStructure& implicitCtStructure,
                                        ImplicitStructureCombination::OperatorType operatorType);

    void CombineWithImplicitCtStructure(ImplicitCtStructureDetails& implicitCtStructureDetails);

    void RefineWithImplicitStructure(const ImplicitCtStructureDetails& newStructureDetails,
                                     ImplicitCtStructure& structureToRefine,
                                     ImplicitStructureCombination::OperatorType operatorType = ImplicitStructureCombination::OperatorType::UNION);

    void RemoveImplicitCtStructure(ImplicitCtStructure& implicitCtStructure);

    CtStructure* GetRoot() const;

    inline void EvaluateAtPosition(const double x[3], CtStructure::Result& result);

    inline CtStructure::ModelingResult FunctionValueAndRadiodensity(const double x[3]);

    void SetData(CtStructure* ctStructure, const QVariant& data);

    CtDataCsgTree(const CtDataCsgTree&) = delete;
    void operator=(const CtDataCsgTree&) = delete;

protected:
    CtDataCsgTree();
    ~CtDataCsgTree() override;

    CtStructure* Root;

private:
    bool CtStructureExists(const CtStructure& ctStructure);
};

void CtDataCsgTree::EvaluateAtPosition(const double x[3], CtStructure::Result &result) {
    if (!Root) {
        qWarning("Tree does not have a root. Cannot evaluate");
        return;
    }

    return Root->EvaluateAtPosition(x, result);
}

inline CtStructure::ModelingResult CtDataCsgTree::FunctionValueAndRadiodensity(const double x[3]) {
    if (!Root) {
        qWarning("Tree does not have a root. Cannot evaluate");
        return {};
    }

    return Root->EvaluateImplicitModel(x);
}
