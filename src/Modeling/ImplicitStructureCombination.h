#pragma once

#include "ImplicitCtStructure.h"

class ImplicitStructureCombination : public CtStructure {
    Q_GADGET

public:
    static ImplicitStructureCombination* New();
    vtkTypeMacro(ImplicitStructureCombination, CtStructure)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    enum OperatorType {
        UNION,
        INTERSECTION,
        DIFFERENCE
    };
    Q_ENUM(OperatorType);
    static std::string OperatorTypeToString(OperatorType operatorType);
    GET_ENUM_VALUES(OperatorType);

    void SetOperatorType(OperatorType operatorType);
    OperatorType GetOperatorType() const;

    void SetTransform(const std::array<std::array<float, 3>, 3>& trs) override;

    void EvaluateAtPosition(const double x[3], Result& result) override;

    const FunctionValueRadiodensity FunctionValueAndRadiodensity(const double x[3]) const override;

    float FunctionValue(const double* x) const override;

    void AddCtStructure(CtStructure& ctStructure);

    CtStructure* RemoveImplicitCtStructure(ImplicitCtStructure* implicitStructure,
                                           ImplicitStructureCombination* grandParent);

    bool CtStructureExists(const CtStructure* structure) override;

    int ChildCount() const override;

    const CtStructure* ChildAt(int idx) const override;

    const std::vector<CtStructure*>* GetChildren() const override;

    QVariant Data() const override;

    void SetData(const QVariant &variant) override;

    bool IsImplicitCtStructure() const override;

    void ReplaceChild(ImplicitCtStructure *oldChild, ImplicitStructureCombination *newChild);

    ImplicitStructureCombination(const ImplicitStructureCombination&) = delete;
    void operator=(const ImplicitStructureCombination&) = delete;

protected:
    ImplicitStructureCombination();
    ~ImplicitStructureCombination() override;

    std::string GetViewName() const override;

    OperatorType OpType;
    std::vector<CtStructure*> CtStructures;

private:
    std::string GetOperatorTypeName() const;

    void ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer);
};

struct ImplicitStructureCombinationDetails : public CtStructureDetails {
    ImplicitStructureCombination::OperatorType OperatorType = ImplicitStructureCombination::OperatorType::UNION;
};
