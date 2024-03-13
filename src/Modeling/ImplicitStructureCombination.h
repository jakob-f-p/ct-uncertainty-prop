#pragma once

#include "ImplicitCtStructure.h"

class ImplicitStructureCombination : public CtStructure {
public:
    static ImplicitStructureCombination* New();
    vtkTypeMacro(ImplicitStructureCombination, CtStructure)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    enum OperatorType {
        UNION,
        INTERSECTION,
        DIFFERENCE
    };

    vtkMTimeType GetMTime() override;

    void SetTransform(vtkTransform* transform) override;

    vtkTransform* GetTransform() override;

    void EvaluateAtPosition(const double x[3], Result& result) override;

    float FunctionValue(const double* x) override;

    void AddCtStructure(CtStructure& ctStructure);

    CtStructure* RemoveImplicitCtStructure(ImplicitCtStructure* implicitStructure,
                                           ImplicitStructureCombination* grandParent);

    void SetOperatorType(OperatorType operatorType);
    OperatorType GetOperatorType();

    bool CtStructureExists(const CtStructure* structure) override;

    int ChildCount() const override;

    int ColumnCount() const override;

    const CtStructure* ChildAt(int idx) const override;

    const std::vector<CtStructure*>& GetChildren() const override;

    QVariant Data(int idx) const override;

    ImplicitStructureCombination(const ImplicitStructureCombination&) = delete;
    void operator=(const ImplicitStructureCombination&) = delete;

protected:
    ImplicitStructureCombination();
    ~ImplicitStructureCombination() override;

    OperatorType OpType;
    std::vector<CtStructure*> CtStructures;
    vtkTransform* Transform;

private:
    const char* GetOperatorTypeName() const;

    void ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer);
};
