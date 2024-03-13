#pragma once

#include "SimpleTransform.h"
#include "../Artifacts/Artifact.h"

#include <QVariant>

#include <vtkObject.h>

/**
 * QDataModel columns:
 *  0: item subtype QString (operator type / implicit function type)
 *  1: item name defined by user QString
 *  2: item details QString
 *  3: long name QString
 *  4: Transform QList<QList<float>> where (0: translate, 1: rotate, 2: scale)
 *  ImplicitCtStructure:
 *    5: Tissue or Material Type QMap<QString, float>
 *    6: Structure Artifacts ?
 *  ImplicitStructureCombination
 *    7: Operator Type QMap<QString, OperatorType>
 */
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

    enum Column {
        SUBTYPE = 0,
        NAME,
        DETAILS,
        LONG_NAME,
        TRANSFORM,
        TISSUE_TYPE,
        STRUCTURE_ARTIFACTS,
        OPERATOR_TYPE,
        NUMBER_OF_COLUMNS
    };

    virtual int ChildCount() const = 0;

    static int ColumnCount() ;

    CtStructure* GetParent() const;

    void SetParent(CtStructure* parent);

    int ChildIndex() const;

    virtual const std::vector<CtStructure*>& GetChildren() const = 0;

    virtual const CtStructure* ChildAt(int idx) const = 0;

    virtual QVariant Data(int idx) const = 0;

    CtStructure(const CtStructure&) = delete;
    void operator=(const CtStructure&) = delete;

protected:
    CtStructure();
    ~CtStructure() override;

    std::string Name;
    SimpleTransform* Transform;
    CtStructure* Parent;    // always of type implicit structure combination
};
