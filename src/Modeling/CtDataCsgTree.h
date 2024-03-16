#pragma once

#include "ImplicitStructureCombination.h"

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

    CtDataCsgTree(const CtDataCsgTree&) = delete;
    void operator=(const CtDataCsgTree&) = delete;

protected:
    CtDataCsgTree();
    ~CtDataCsgTree() override;

    CtStructure* Root;

private:
    bool CtStructureExists(const CtStructure& ctStructure);
};
