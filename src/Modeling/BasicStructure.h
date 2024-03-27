#pragma once

#include "CtStructure.h"
#include "../Enum.h"

#include <QMetaEnum>

#include <vtkImplicitFunction.h>

struct BasicStructureDetails;

/**
 * @class BasicStructure
 * @brief class representing implicit CT structures with structure Artifacts
 *
 * This class is used in CtStructureTree as an implicit source of data.
 */
class BasicStructure : public CtStructure {
    Q_GADGET

public:
    static BasicStructure* New();
    vtkTypeMacro(BasicStructure, CtStructure);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    enum ImplicitFunctionType {
        SPHERE,
        BOX,
        CONE,
        INVALID
    };
    Q_ENUM(ImplicitFunctionType);
    static std::string ImplicitFunctionTypeToString(ImplicitFunctionType implicitFunctionType);
    GET_ENUM_VALUES(ImplicitFunctionType, true);

    /**
     * Set/Get the implicit function separating the function domain into position inside, on, and outside of the
     * surface. The domain on and inside the surface (f(x,y,z) <= 0) constitutes the structure.
     */
    void SetImplicitFunction(ImplicitFunctionType implicitFunctionType);

    struct TissueOrMaterialType {
        std::string Name;
        float CtNumber = 0.0f; // value on the Hounsfield scale

        friend std::ostream& operator<< (std::ostream& stream, const TissueOrMaterialType& type);
    };
    static TissueOrMaterialType GetTissueOrMaterialTypeByName(const std::string& tissueName);
    static QStringList GetTissueAndMaterialTypeNames();

    void SetTransform(const std::array<std::array<float, 3>, 3>& trs) override;

    /**
     * Set the type of tissue.
     */
    void SetTissueType(TissueOrMaterialType tissueType);

    void EvaluateAtPosition(const double x[3], Result& result) override;

    const ModelingResult EvaluateImplicitModel(const double x[3]) const override;

    float FunctionValue(const double x[3]) const override;

    bool CtStructureExists(const CtStructure* structure) override;

    QVariant Data() const override;

    void SetData(const QVariant& variant) override;
    void SetData(const BasicStructureDetails& basicStructureDetails);

    SubType GetSubType() const override;

    static QWidget* GetEditWidget(ImplicitFunctionType functionType);
    static void SetEditWidgetData(QWidget* widget, const BasicStructureDetails& basicStructureDetails);
    static BasicStructureDetails GetEditWidgetData(QWidget* widget);

    void DeepCopy(CtStructure* source, CombinedStructure* parent) override;

    BasicStructure(const BasicStructure&) = delete;
    void operator=(const BasicStructure&) = delete;

protected:
    BasicStructure();
    ~BasicStructure() override;

    std::string GetViewName() const override;

    int Id;
    ImplicitFunctionType ImplicitFType;
    vtkImplicitFunction* ImplicitFunction;
    TissueOrMaterialType Tissue;

    static std::map<std::string, TissueOrMaterialType> TissueTypeMap;
    static std::atomic<uint16_t> GlobalBasicStructureId;

private:
    static QString FunctionTypeComboBoxName;
    static QString TissueTypeComboBoxName;
};

struct BasicStructureDetails : public CtStructureDetails {
    BasicStructure::ImplicitFunctionType ImplicitFunctionType = BasicStructure::ImplicitFunctionType::SPHERE;
    QString TissueName;
};

