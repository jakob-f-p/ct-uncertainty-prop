#pragma once

#include "ImplicitCtStructure.h"

class ImplicitStructureCombination : public CtStructure {
public:
    static ImplicitStructureCombination* New();
    vtkTypeMacro(ImplicitStructureCombination, CtStructure)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    enum OperatorType {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        NUMBER_OF_OPERATOR_TYPES
    };
    static std::string OperatorTypeToString(OperatorType operatorType);
    static OperatorType StringToOperatorType(const std::string& string);

    void SetTransform(const QVariant& trs) override;

    void EvaluateAtPosition(const double x[3], Result& result) override;

    float FunctionValue(const double* x) override;

    void AddCtStructure(CtStructure& ctStructure);

    CtStructure* RemoveImplicitCtStructure(ImplicitCtStructure* implicitStructure,
                                           ImplicitStructureCombination* grandParent);

    void SetOperatorType(OperatorType operatorType);
    OperatorType GetOperatorType() const;

    bool CtStructureExists(const CtStructure* structure) override;

    int ChildCount() const override;

    const CtStructure* ChildAt(int idx) const override;

    const std::vector<CtStructure*>* GetChildren() const override;

    QVariant PackageData(DataKey dataKey) const override;

    ImplicitStructureCombination(const ImplicitStructureCombination&) = delete;
    void operator=(const ImplicitStructureCombination&) = delete;

protected:
    ImplicitStructureCombination();
    ~ImplicitStructureCombination() override;

    OperatorType OpType;
    std::vector<CtStructure*> CtStructures;

private:
    std::string GetOperatorTypeName() const;

    void ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer);
};
