#pragma once

#include "CtStructure.h"

class BasicStructure;

struct CombinedStructureDetails;

class CombinedStructure : public CtStructure {
    Q_GADGET

public:
    static CombinedStructure* New();
    vtkTypeMacro(CombinedStructure, CtStructure)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    enum OperatorType {
        UNION,
        INTERSECTION,
        DIFFERENCE
    };
    Q_ENUM(OperatorType);
    static std::string OperatorTypeToString(OperatorType operatorType);
    GET_ENUM_VALUES(OperatorType, false);

    void SetOperatorType(OperatorType operatorType);
    OperatorType GetOperatorType() const;

    void SetTransform(const std::array<std::array<float, 3>, 3>& trs) override;

    void EvaluateAtPosition(const double x[3], Result& result) override;

    const ModelingResult EvaluateImplicitModel(const double x[3]) const override;

    float FunctionValue(const double* x) const override;

    void AddCtStructure(CtStructure& ctStructure);

    CtStructure* RemoveBasicStructure(BasicStructure* basicStructure,
                                      CombinedStructure* grandParent);

    bool CtStructureExists(const CtStructure* structure) override;

    int ChildCount() const;

    const CtStructure* ChildAt(int idx) const;

    int ChildIndex(const CombinedStructure& child) const;

    QVariant Data() const override;

    void SetData(const QVariant &variant) override;

    SubType GetSubType() const override;

    void ReplaceChild(BasicStructure *oldChild, CombinedStructure *newChild);

    static QWidget* GetEditWidget();
    static void SetEditWidgetData(QWidget* widget, const CombinedStructureDetails& combinedStructureDetails);
    static CombinedStructureDetails GetEditWidgetData(QWidget* widget);

    void DeepCopy(CtStructure* source, CombinedStructure* parent) override;

    CombinedStructure(const CombinedStructure&) = delete;
    void operator=(const CombinedStructure&) = delete;

protected:
    CombinedStructure();
    ~CombinedStructure() override;

    std::string GetViewName() const override;

    OperatorType OpType;
    std::vector<CtStructure*> CtStructures;

private:
    std::string GetOperatorTypeName() const;

    void ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer);

    static QString OperatorTypeComboBoxName;
};

struct CombinedStructureDetails : public CtStructureDetails {
    CombinedStructure::OperatorType OperatorType = CombinedStructure::OperatorType::UNION;
};
