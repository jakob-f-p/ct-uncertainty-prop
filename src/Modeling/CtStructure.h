#pragma once

#include "SimpleTransform.h"
#include "../Artifacts/Artifact.h"
#include "../Artifacts/StructureArtifactList.h"

#include <QVariant>

#include <vtkObject.h>
#include <vtkImplicitFunction.h>

class CtStructure : public vtkObject {
public:
    vtkTypeMacro(CtStructure, vtkObject)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    void SetName(std::string name);
    std::string GetName() const;

    virtual void SetTransform(const QVariant& trs) = 0;
    const SimpleTransform* GetTransform() const;
    QVariant GetTransformQVariant() const;

    struct Result {
        float FunctionValue;
        float IntensityValue;
        std::map<Artifact::SubType, float> ArtifactValueMap;
    };

    virtual void EvaluateAtPosition(const double x[3], Result& result) = 0;
    /**
     * Return f(x, y z) where f > 0 is outside of the surface, f = 0 is on the surface, and f < 0 is inside the surface.
     * Additionally, the distance to the surface is positively correlated with the function value at a given position.
     * Function takes the transform of the object into account.
     * @param x the input position vector consisting of x, y, and z coordinates
     * @return the function value f(x, y, z) at position x
     */
    virtual float FunctionValue(const double x[3]) = 0;

    virtual bool CtStructureExists(const CtStructure* structure) = 0;

    enum DataKey {
        STRUCTURE_TYPE,
        NAME,
        EDIT_DIALOG_NAME,
        TREE_VIEW_NAME,
        TRANSFORM,
        IMPLICIT_FUNCTION_TYPE,
        TISSUE_TYPE,
        STRUCTURE_ARTIFACTS,
        OPERATOR_TYPE,
        NUMBER_OF_DATA_KEYS
    };
    static std::string DataKeyToString(DataKey dataKey);
    static DataKey StringToDataKey(const std::string& string);

    virtual int ChildCount() const = 0;

    static int ColumnCount() ;

    CtStructure* GetParent() const;

    void SetParent(CtStructure* parent);

    int ChildIndex() const;

    virtual const std::vector<CtStructure*>* GetChildren() const = 0;

    virtual const CtStructure* ChildAt(int idx) const = 0;

    QVariant Data() const;

    virtual QVariant PackageData(DataKey dataKey) const = 0;

    CtStructure(const CtStructure&) = delete;
    void operator=(const CtStructure&) = delete;

protected:
    CtStructure();
    ~CtStructure() override;

    std::string Name;
    SimpleTransform* Transform;
    CtStructure* Parent;    // always of type implicit structure combination
};
