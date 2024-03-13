#pragma once

#include "../Artifacts/Artifact.h"

#include <QVariant>

#include <vtkTransform.h>
#include <vtkObject.h>

class CtStructure : public vtkObject {
public:
    vtkTypeMacro(CtStructure, vtkObject)

    void PrintSelf(ostream& os, vtkIndent indent) override;

    virtual void SetTransform(vtkTransform* transform) = 0;

    virtual vtkTransform* GetTransform() = 0;

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

    virtual int ChildCount() const = 0;

    virtual int ColumnCount() const = 0;

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
    ~CtStructure() override = default;

    CtStructure* Parent;    // always of type implicit structure combination
};
