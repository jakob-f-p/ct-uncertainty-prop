#pragma once

#include "CtStructure.h"

#include "../Artifacts/StructureArtifactList.h"

#include <vtkImplicitFunction.h>


/**
 * @class ImplicitCtStructure
 * @brief class representing implicit CT structures with structure Artifacts
 *
 * This class is used in ImplicitCsgTree as an implicit source of data.
 */
class ImplicitCtStructure : public CtStructure {
public:
    static ImplicitCtStructure* New();
    vtkTypeMacro(ImplicitCtStructure, CtStructure);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    enum ImplicitFunctionType {
        SPHERE,
        BOX,
        CONE,
        NUMBER_OF_IMPLICIT_FUNCTION_TYPES
    };

    static std::string ImplicitFunctionTypeToString(ImplicitFunctionType implicitFunctionType);
    static ImplicitFunctionType StringToImplicitFunctionType(const std::string& string);

    /**
     * Set/Get the implicit function separating the function domain into position inside, on, and outside of the
     * surface. The domain on and inside the surface (f(x,y,z) <= 0) constitutes the structure.
     */
    void SetImplicitFunction(ImplicitFunctionType implicitFunctionType);

    struct TissueOrMaterialType {
        std::string Name;
        float CtNumber; // value on the Hounsfield scale

        friend std::ostream& operator<< (std::ostream& stream, const TissueOrMaterialType& type);
    };
    static TissueOrMaterialType GetTissueOrMaterialTypeByName(const std::string& tissueName);
    static std::vector<std::string> GetTissueAndMaterialTypeNames();
    static QStringList GetTissueAndMaterialTypeNamesQ();

    void SetTransform(const QVariant& trs) override;

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

    QVariant PackageData(DataKey dataKey) const override;

    ImplicitCtStructure(const ImplicitCtStructure&) = delete;
    void operator=(const ImplicitCtStructure&) = delete;

protected:
    ImplicitCtStructure();
    ~ImplicitCtStructure() override;

    ImplicitFunctionType ImplicitFType;
    vtkImplicitFunction* ImplicitFunction;
    TissueOrMaterialType Tissue;
    StructureArtifactList* StructureArtifacts;

    static std::map<std::string, TissueOrMaterialType> TissueTypeMap;
};