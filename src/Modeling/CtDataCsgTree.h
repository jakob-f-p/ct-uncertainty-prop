#pragma once

#include "CtStructure.h"
#include "ImplicitCtStructure.h"
#include "ImplicitStructureCombination.h"

class CtDataCsgTree : public vtkObject {
public:
    static CtDataCsgTree* New();
    vtkTypeMacro(CtDataCsgTree, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void AddImplicitCtStructure(ImplicitCtStructure& implicitCtStructure,
                                ImplicitStructureCombination* parent = nullptr);

    void CombineWithImplicitCtStructure(ImplicitCtStructure& implicitCtStructure,
                                        ImplicitStructureCombination::OperatorType operatorType);

    void RemoveImplicitCtStructure(ImplicitCtStructure& implicitCtStructure);

    CtDataCsgTree(const CtDataCsgTree&) = delete;
    void operator=(const CtDataCsgTree&) = delete;

protected:
    CtDataCsgTree();
    ~CtDataCsgTree() override;

    CtStructure* Root;

private:
    bool CtStructureExists (const CtStructure& ctStructure);
};
