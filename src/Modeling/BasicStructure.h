#pragma once

#include "CtStructure.h"
#include "../Enum.h"

#include <QFormLayout>

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

    void SetTransform(const std::array<std::array<float, 3>, 3>& trs) override;

    enum class ImplicitFunctionType {
        SPHERE,
        BOX,
        CONE,
        INVALID
    };
    Q_ENUM(ImplicitFunctionType);
    static std::string ImplicitFunctionTypeToString(ImplicitFunctionType implicitFunctionType);
    GET_ENUM_VALUES(ImplicitFunctionType, true);
    ImplicitFunctionType GetFunctionType() const;
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
    void SetTissueType(TissueOrMaterialType tissueType);

    void EvaluateAtPosition(const double x[3], Result& result) override;

    const ModelingResult EvaluateImplicitModel(const double x[3]) const override;

    float FunctionValue(const double x[3]) const override;

    bool CtStructureExists(const CtStructure* structure) override;

    SubType GetSubType() const override;

    void DeepCopy(CtStructure* source, CombinedStructure* parent) override;

    BasicStructure(const BasicStructure&) = delete;
    void operator=(const BasicStructure&) = delete;

protected:
    BasicStructure();
    ~BasicStructure() override;

    friend struct BasicStructureData;

    std::string GetViewName() const override;

    int Id;
    ImplicitFunctionType FunctionType;
    vtkImplicitFunction* ImplicitFunction;
    TissueOrMaterialType Tissue;

    static std::map<std::string, TissueOrMaterialType> TissueTypeMap;
    static std::atomic<uint16_t> GlobalBasicStructureId;
};


struct BasicStructureData : public CtStructureData<BasicStructure, BasicStructureData> {
    struct SphereData {
        double Radius;
        std::array<double, 3> Center;
    };

    struct BoxData {
        std::array<double, 3> MinPoint;
        std::array<double, 3> MaxPoint;
    };

    BasicStructure::ImplicitFunctionType FunctionType = BasicStructure::ImplicitFunctionType::INVALID;
    QString TissueName;
    SphereData Sphere {};
    BoxData Box {};

protected:
    friend struct CtStructureData<BasicStructure, BasicStructureData>;

    static void AddDerivedData(const BasicStructure& structure, BasicStructureData& data);

    static void SetDerivedData(BasicStructure& structure, const BasicStructureData& data);
};


class BasicStructureUi : public CtStructureUi<BasicStructureUi, BasicStructureData> {
public:
    static void SetFunctionType(QWidget* widget, BasicStructure::ImplicitFunctionType functionType);

protected:
    friend struct CtStructureUi<BasicStructureUi, BasicStructureData>;

    using CtStructureUi = CtStructureUi<BasicStructureUi, BasicStructureData>;

    static void AddDerivedWidgets(QFormLayout* fLayout);

    static void AddDerivedWidgetsData(QWidget* widget, BasicStructureData& data);

    static void SetDerivedWidgetsData(QWidget* widget, const BasicStructureData& data);

private:
    static QGroupBox* GetFunctionParametersGroup(BasicStructure::ImplicitFunctionType functionType);

    static void UpdateFunctionParametersGroup(QFormLayout* fLayout);

    static const QString FunctionTypeComboBoxName;
    static const QString TissueTypeComboBoxName;
    static const QString FunctionParametersGroupName;
    static const QString SphereRadiusSpinBoxName;
    static const QString SphereCenterName;
    static const QString BoxMinPointName;
    static const QString BoxMaxPointName;
};
