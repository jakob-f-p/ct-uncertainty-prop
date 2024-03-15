#pragma once

#include "CtStructure.h"
#include "../Artifacts/StructureArtifactList.h"

#include <QMetaEnum>

#include <vtkImplicitFunction.h>

/**
 * @class ImplicitCtStructure
 * @brief class representing implicit CT structures with structure Artifacts
 *
 * This class is used in ImplicitCsgTree as an implicit source of data.
 */
class ImplicitCtStructure : public CtStructure {
    Q_GADGET

public:
    static ImplicitCtStructure* New();
    vtkTypeMacro(ImplicitCtStructure, CtStructure);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    enum ImplicitFunctionType {
        SPHERE,
        BOX,
        CONE
    };
    Q_ENUM(ImplicitFunctionType);
    static std::string ImplicitFunctionTypeToString(ImplicitFunctionType implicitFunctionType);
    GET_ENUM_VALUES(ImplicitFunctionType);

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

    float FunctionValue(const double x[3]) override;

    bool CtStructureExists(const CtStructure* structure) override;

    int ChildCount() const override;

    const std::vector<CtStructure*>* GetChildren() const override;

    const CtStructure* ChildAt(int idx) const override;

    QVariant Data() const override;

public:
    ImplicitCtStructure(const ImplicitCtStructure&) = delete;
    void operator=(const ImplicitCtStructure&) = delete;

protected:
    ImplicitCtStructure();
    ~ImplicitCtStructure() override;

    std::string GetViewName() const override;

    ImplicitFunctionType ImplicitFType;
    vtkImplicitFunction* ImplicitFunction;
    TissueOrMaterialType Tissue;
    StructureArtifactList* StructureArtifacts;

    static std::map<std::string, TissueOrMaterialType> TissueTypeMap;
};

struct ImplicitCtStructureDetails : public CtStructureDetails {
    ImplicitCtStructure::ImplicitFunctionType ImplicitFunctionType = ImplicitCtStructure::ImplicitFunctionType::SPHERE;
    QString TissueName;
    QList<StructureArtifactDetails> StructureArtifacts;
};
