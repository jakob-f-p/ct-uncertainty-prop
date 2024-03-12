#pragma once

#include "ImplicitCtStructure.h"

class ImplicitStructureCombination : public CtStructure {

public:
    static ImplicitStructureCombination* New();
    vtkTypeMacro(ImplicitStructureCombination, CtStructure)


    void PrintSelf(ostream& os, vtkIndent indent) override;

    void Delete() override;

    enum OperatorType {
        UNION,
        INTERSECTION,
        DIFFERENCE
    };

    vtkMTimeType GetMTime() override;

    void SetTransform(vtkAbstractTransform* transform) override;

    vtkAbstractTransform* GetTransform() override;

    void EvaluateAtPosition(const double x[3], Result& result) override;

    float FunctionValue(const double* x) override;

    void AddCtStructure(CtStructure& ctStructure);

    CtStructure* RemoveImplicitCtStructure(ImplicitCtStructure* implicitStructure,
                                           ImplicitStructureCombination* grandParent);

    ImplicitStructureCombination* FindParentOfCtStructure(CtStructure& ctStructure);

    void SetOperatorType(OperatorType operatorType);
    OperatorType GetOperatorType();

    bool CtStructureExists(const CtStructure* structure) override;

    size_t GetNumberOfChildStructures();

    ImplicitStructureCombination(const ImplicitStructureCombination&) = delete;
    void operator=(const ImplicitStructureCombination&) = delete;

protected:
    ImplicitStructureCombination();
    ~ImplicitStructureCombination() override;

    OperatorType OpType;
    std::vector<CtStructure*> CtStructures;
    vtkAbstractTransform* Transform;

private:
    const char* GetOperatorTypeName();

    void ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer);
};
