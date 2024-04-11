#pragma once

//#include "CtStructure.h"
#include "BasicStructure.h"

#include <QFormLayout>

class BasicStructure;

class CombinedStructure : public CtStructure {
    Q_GADGET

public:
    static CombinedStructure* New();
    vtkTypeMacro(CombinedStructure, CtStructure)
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void SetTransform(const std::array<std::array<float, 3>, 3>& trs) override;

    SubType GetSubType() const override;

    enum class OperatorType {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        INVALID
    };
    Q_ENUM(OperatorType);
    static std::string OperatorTypeToString(OperatorType operatorType);
    GET_ENUM_VALUES(OperatorType, true);
    void SetOperatorType(OperatorType operatorType);

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

    void ReplaceChild(BasicStructure* oldChild, CombinedStructure* newChild);

    void Iterate(const std::function<void(CtStructure&)>& f) override;

    CombinedStructure(const CombinedStructure&) = delete;
    void operator=(const CombinedStructure&) = delete;

protected:
    CombinedStructure() = default;
    ~CombinedStructure() override = default;

    friend struct CombinedStructureData;

    std::string GetViewName() const override;

    using CtStructureList = std::vector<vtkSmartPointer<CtStructure>>;

    OperatorType Operator = OperatorType::INVALID;
    CtStructureList ChildStructures;

private:
    std::string GetOperatorTypeName() const;

    void ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer);
};


struct CombinedStructureData : public CtStructureData<CombinedStructure, CombinedStructureData> {
    CombinedStructure::OperatorType Operator = CombinedStructure::OperatorType::INVALID;

    CombinedStructureData() = default;

protected:
    friend struct CtStructureData<CombinedStructure, CombinedStructureData>;

    static void AddSubTypeData(const CombinedStructure& structure, CombinedStructureData& data);

    static void SetSubTypeData(CombinedStructure& structure, const CombinedStructureData& data);
};


class CombinedStructureUi : public CtStructureUi<CombinedStructureUi, CombinedStructureData> {
protected:
    friend struct CtStructureUi<CombinedStructureUi, CombinedStructureData>;

    static void AddSubTypeWidgets(QFormLayout* fLayout);

    static void AddSubTypeWidgetsData(QWidget* widget, CombinedStructureData& data);

    static void SetSubTypeWidgetsData(QWidget* widget, const CombinedStructureData& data);

private:
    static const QString OperatorTypeComboBoxName;
};
